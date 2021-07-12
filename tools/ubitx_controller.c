/* ubitx_controller
 * Copyright (C) 2021 Rhizomatica
 * Author: Rafael Diniz <rafael@riseup.net>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <threads.h>
#include <pthread.h>

#include "serial.h"
#include "ubitx_controller.h"
#include "../common/radio_cmds.h"

#include "shm.h"

static bool running;

int cat_tester(void *arg)
{
    int *target_fd_ptr = (int *)arg;
    int target_fd = *target_fd_ptr;
    char buf[MAX_BUF_SIZE];

    if (thrd_detach(thrd_current()) != thrd_success) {
        /* Handle error */
    }

    fd_set fds, fds1;
    int i, cc, max;

    FD_ZERO(&fds);
    FD_SET(target_fd, &fds);
    max = target_fd + 1;

    while (running) {
        memcpy(&fds1, &fds, sizeof(fd_set));
        i = select(max, &fds1, NULL, NULL, NULL);
        if (i < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "select() error\n");
            exit(1);
        }

        if (FD_ISSET(target_fd, &fds1)) {

            cc = read(target_fd, buf, 1);
            if (cc <= 0) {
                fprintf(stderr, "EOF/error on target tty\n");
                exit(1);
            }
            buf[cc] = 0;
            fprintf(stderr, "read byte 0x%hhx\n", buf[0]);
        }

    }

    return EXIT_SUCCESS;

}

int cat_tx(void *arg)
{
    controller_conn *conn = arg;

    pthread_mutex_lock(&conn->ptt_mutex);

    while(running)
    {
        pthread_cond_wait(&conn->ptt_condition, &conn->ptt_mutex);
        write(conn->radio_fd, conn->service_command, 5);
        // fprintf(stderr,"Sent to the radio:  0x%hhx\n", conn->service_command[0]);
    }

    pthread_mutex_unlock(&conn->ptt_mutex);

    return EXIT_SUCCESS;
}

int cat_rcv(void *arg)
{
    controller_conn *conn = arg;
    int target_fd = conn->radio_fd;
    uint8_t buf[MAX_BUF_SIZE];

    fd_set fds, fds1;
    int i, cc, max;

    FD_ZERO(&fds);
    FD_SET(target_fd, &fds);
    max = target_fd + 1;

    while (running) {
        memcpy(&fds1, &fds, sizeof(fd_set));
        i = select(max, &fds1, NULL, NULL, NULL);
        if (i < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "select() error\n");
            exit(1);
        }

        if (FD_ISSET(target_fd, &fds1)) {

            cc = read(target_fd, buf, 1);
            if (cc <= 0) {
                fprintf(stderr, "EOF/error on target tty\n");
                exit(1);
            }
            fprintf(stderr, "RESP BYTE: 0x%hhx\n", buf[0]);


            switch(buf[0])
            {
                // ptt responses...
            case CMD_RESP_PTT_ON_ACK:
            case CMD_RESP_PTT_ON_NACK:
            case CMD_RESP_PTT_OFF_ACK:
            case CMD_RESP_PTT_OFF_NACK:
                conn->ptt_last_response = buf[0];
                continue;
                break;

            case CMD_ALERT_PROTECTION_ON:
                conn->protection_alert = buf[0];
                continue;
                break;

            case CMD_RESP_WRONG_COMMAND:
                // continue;
                // break;

                // 1 byte responses
            case CMD_RESP_SET_FREQ_ACK:
            case CMD_RESP_SET_MODE_ACK:
            case CMD_RESP_GET_MODE_USB:
            case CMD_RESP_GET_MODE_LSB:
            case CMD_RESP_GET_TXRX_INTX:
            case CMD_RESP_GET_TXRX_INRX:
            case CMD_RESP_GET_PROTECTION_ON:
            case CMD_RESP_GET_PROTECTION_OFF:
            case CMD_RESP_SET_MASTERCAL_ACK:
            case CMD_RESP_SET_BFO_ACK:
            case CMD_RESP_GET_LED_STATUS_ON:
            case CMD_RESP_GET_LED_STATUS_OFF:
            case CMD_RESP_SET_LED_STATUS_ACK:
            case CMD_RESP_GET_BYPASS_STATUS_ON:
            case CMD_RESP_GET_BYPASS_STATUS_OFF:
            case CMD_RESP_SET_BYPASS_STATUS_ACK:
            case CMD_RESP_SET_SERIAL_ACK:
            case CMD_RESP_RESET_PROTECTION_ACK:
                conn->response_service_type = CMD_RESP_SHORT;
                conn->response_service[0] = buf[0];
                break;

                // 5 bytes commands
            case CMD_RESP_GET_FREQ_ACK:
            case CMD_RESP_GET_MASTERCAL_ACK:
            case CMD_RESP_GET_BFO_ACK:
            case CMD_RESP_GET_FWD_ACK:
            case CMD_RESP_GET_REF_ACK:
            case CMD_RESP_GET_SERIAL_ACK:
                conn->response_service_type = CMD_RESP_LONG;
                fprintf(stderr, "Is long answer.\n");
                cc = read(target_fd, buf+1, 4);
                if (cc != 4) {
                    // fix this - store the data read in a buffer!
                    fprintf(stderr, "BAD!\n");
                    continue;
                }
                memcpy(conn->response_service, buf, 5);
                break;

            default:
                fprintf(stderr, "Response not recognized: this is not supposed to happen!\n");
                continue;

            }
            conn->response_available = 1;
        }
    }

    return EXIT_SUCCESS;
}

bool initialize_message(controller_conn *connector)
{
    pthread_mutex_t *mutex_ptr = (pthread_mutex_t *) & connector->ptt_mutex;
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr)) {
        perror("pthread_mutexattr_init");
        return false;
    }
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_mutexattr_setpshared");
        return false;
    }

    if (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)){
        perror("pthread_mutexattr_setrobust");
        return false;
    }

    if (pthread_mutex_init(mutex_ptr, &attr)) {
        perror("pthread_mutex_init");
        return false;
    }

    if (pthread_mutexattr_destroy(&attr)) {
        perror("pthread_mutexattr_destroy");
        return false;
    }

    // init the cond and mutex
    pthread_cond_t *cond_ptr = (pthread_cond_t *) & connector->ptt_condition;
    pthread_condattr_t cond_attr;
    if (pthread_condattr_init(&cond_attr)) {
        perror("pthread_condattr_init");
        return false;
    }
    if (pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_condattr_setpshared");
        return false;
    }

    if (pthread_cond_init(cond_ptr, &cond_attr)) {
        perror("pthread_cond_init");
        return false;
    }

    if (pthread_condattr_destroy(&cond_attr)) {
        perror("pthread_condattr_destroy");
        return false;
    }

    connector->radio_fd = -1;
    connector->response_available = 0;
    connector->protection_alert = 0;
    connector->ptt_last_response = CMD_RESP_PTT_OFF_ACK;

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int radio_type = RADIO_TYPE_UBITX;
    char serial_path[MAX_MODEM_PATH];
    bool cat_tester_mode = false;

    if (argc < 3)
    {
    manual:
        fprintf(stderr, "Usage modes: \n%s -s serial_device -r [icom,ubitx]\n", argv[0]);
        fprintf(stderr, "%s -h\n", argv[0]);
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, " -s serial_device           Set the serial device file path\n");
        fprintf(stderr, " -t                         Run a serial test\n");
        fprintf(stderr, " -r [icom,ubitx]            Sets radio type (supported: icom or ubitx)\n");
        fprintf(stderr, " -h                         Prints this help.\n");
        exit(EXIT_FAILURE);
    }

    controller_conn *connector;

    if (shm_is_created(SYSV_SHM_KEY_STR, sizeof(controller_conn)))
    {
        fprintf(stderr, "Connector SHM is already created!\nDestroying it and creating again.\n");
        shm_destroy(SYSV_SHM_KEY_STR, sizeof(controller_conn));
    }
    shm_create(SYSV_SHM_KEY_STR, sizeof(controller_conn));

    connector = shm_attach(SYSV_SHM_KEY_STR, sizeof(controller_conn));
    // tmp_conn = connector;

    initialize_message(connector);

    int opt;
    while ((opt = getopt(argc, argv, "hs:tr:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            goto manual;
            break;
        case 's':
            strcpy(serial_path, optarg);
            break;
        case 'r':
            // ubitx is the default...
            if (!strcmp(optarg,"icom"))
                radio_type = RADIO_TYPE_ICOM;
            break;
        case 't':
            cat_tester_mode = true;
            break;
        default:
            goto manual;
        }
    }

    connector->radio_fd = open_serial_port(serial_path);

    if (connector->radio_fd == -1)
    {
        fprintf(stderr, "Could not open serial device.\n");
        return EXIT_FAILURE;
    }

    if (radio_type == RADIO_TYPE_ICOM){
        fprintf(stderr, "icom\n");
        set_fixed_baudrate("19200", connector->radio_fd);
    }

    if (radio_type == RADIO_TYPE_UBITX){
        fprintf(stderr, "ubitx\n");
        set_fixed_baudrate("38400", connector->radio_fd);
    }

    // mamma mia!!!!
    sleep(2);

    running = true;

    if (cat_tester_mode)
    {
        thrd_t rx_thread;
        thrd_create(&rx_thread, cat_tester, &connector->radio_fd );

        uint8_t i = 0;
        while(i != 255)
        {
            write(connector->radio_fd, &i, 1);
            sleep(1);
            i++;
        }
        write(connector->radio_fd, &i, 1);

        // first test the serial... what goes through, what goes not...

        fprintf(stderr, "Finished writing.\n");
        // then test all the commands....
        sleep(10);
        return EXIT_SUCCESS;
    }

    thrd_t rx_thread;
    thrd_create(&rx_thread, cat_rcv, connector);

    cat_tx((void *) connector);

    // tx_thread... cat tx

}
