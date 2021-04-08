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

#include "serial.h"
#include "ubitx_controller.h"
#include "../common/radio_cmds.h"

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

}

int cat_rcv(void *arg)
{
    int *target_fd_ptr = (int *)arg;
    int target_fd = *target_fd_ptr;
    uint8_t buf[MAX_BUF_SIZE];

    int frequency;

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
            fprintf(stderr, "RESP BYTE: 0x%hhx\n", buf[0]);
            if (buf[0] <= CMD_LAST_5BYTES)
            {
                fprintf(stderr, "Is long answer.\n");
                cc = read(target_fd, buf+1, 4);
                if (cc != 4) {
                    fprintf(stderr, "BAD!\n");
                    // exit(1);
                }
            }


            switch(buf[0])
            {
                // 1 byte responses
            case CMD_RESP_PTT_ON_ACK:
                // log write ptt on ack
                break;
            case CMD_RESP_PTT_ON_NACK:
                // not good
                break;

            case CMD_RESP_PTT_OFF_ACK:
                // log write ptt on ack
                break;
            case CMD_RESP_PTT_OFF_NACK:
                // not good
                break;

            case CMD_RESP_SET_FREQ_ACK:
                // log
                break;

            case CMD_RESP_SET_MODE_ACK:
                // log
                break;

            case CMD_RESP_GET_MODE_USB:
                // log
                break;

            case CMD_RESP_GET_MODE_LSB:
                // log
                break;

            case CMD_RESP_GET_TXRX_INTX:
                // log
                break;

            case CMD_RESP_GET_TXRX_INRX:
                // log
                break;

            case CMD_RESP_GET_PROTECTION_ON:
                // log
                break;
            case CMD_RESP_GET_PROTECTION_OFF:
                break;
            case CMD_RESP_SET_MASTERCAL_ACK:
                break;
            case CMD_RESP_SET_BFO_ACK:
                break;
            case CMD_RESP_GET_LED_STATUS_ON:
                break;
            case CMD_RESP_GET_LED_STATUS_OFF:
                break;
            case CMD_RESP_SET_LED_STATUS_ACK:
                break;
            case CMD_RESP_GET_BYPASS_STATUS_ON:
                break;
            case CMD_RESP_GET_BYPASS_STATUS_OFF:
                break;
            case CMD_RESP_SET_BYPASS_STATUS_ACK:
                break;
            case CMD_RESP_WRONG_COMMAND:
                break;
            case CMD_ALERT_PROTECTION_ON:
                break;

                // 5 bytes commands
            case CMD_RESP_GET_FREQ_ACK:
                // memcpy (frequency, buf+1, 4);
                memcpy(&frequency, buf+1, 4);
                fprintf(stderr, "frequency: %d\n", frequency);

                break;
            case CMD_RESP_GET_MASTERCAL_ACK:
                break;
            case CMD_RESP_GET_BFO_ACK:
                break;
            case CMD_RESP_GET_FWD_ACK:
                break;
            case CMD_RESP_GET_REF_ACK:
                break;

            default:
                fprintf(stderr, "this is not supposed to happen\n");

            }

        }

    }
}


int main(int argc, char *argv[])
{
    int ssb_mode = UNDEFINED;
    int radio_type = RADIO_TYPE_ICOM;
    int frequency = 0;
    char serial_path[MAX_MODEM_PATH];
    int serial_fd = -1;
    bool tester_mode = false;
    bool cat_tester_mode = false;

    if (argc < 3)
    {
    manual:
        fprintf(stderr, "Usage modes: \n%s -s serial_device -r [icom,ubitx] -f frequency [-u -l] -c [on,off]\n", argv[0]);
        fprintf(stderr, "%s -h\n", argv[0]);
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, " -s serial_device           Set the serial device file path\n");
        fprintf(stderr, " -f frequency               Sets the dial frequency (in Hz)\n");
        fprintf(stderr, " -c                         Run a serial test\n");
        fprintf(stderr, " -l                         Sets LSB mode\n");
        fprintf(stderr, " -u                         Sets USB mode\n");
        fprintf(stderr, " -r [icom,ubitx]            Sets radio type (supported: icom or ubitx)\n");
        fprintf(stderr, " -t                         Runs the command tester!\n");
        fprintf(stderr, " -h                         Prints this help.\n");
        exit(EXIT_FAILURE);
    }

    int opt;
    while ((opt = getopt(argc, argv, "hs:cf:lur:t")) != -1)
    {
        switch (opt)
        {
        case 'h':
            goto manual;
            break;
        case 't':
            tester_mode = true;
            break;
        case 's':
            strcpy(serial_path, optarg);
            break;
        case 'l':
            ssb_mode = LSB;
            break;
        case 'u':
            ssb_mode = USB;
            break;
        case 'r':
            // icom is the default...
            if (!strcmp(optarg,"ubitx"))
                radio_type = RADIO_TYPE_UBITX;
            break;
        case 'f':
            frequency = atoi(optarg);
            break;
        case 'c':
            cat_tester_mode = true;
            break;
        default:
            goto manual;
        }
    }

    serial_fd = open_serial_port(serial_path);

    if (serial_fd == -1)
    {
        fprintf(stderr, "Could not open serial device.\n");
        return EXIT_FAILURE;
    }

    if (radio_type == RADIO_TYPE_ICOM){
        fprintf(stderr, "icom\n");
        set_fixed_baudrate("19200", serial_fd);
    }

    if (radio_type == RADIO_TYPE_UBITX){
        fprintf(stderr, "ubitx\n");
        set_fixed_baudrate("38400", serial_fd);
    }

    // mamma mia!!!!
    sleep(2);

    running = true;

    if (cat_tester_mode)
    {
        thrd_t rx_thread;
        thrd_create(&rx_thread, cat_tester, &serial_fd );

        uint8_t i = 0;
        while(i != 255)
        {
            write(serial_fd, &i, 1);
            sleep(1);
            i++;
        }
        write(serial_fd, &i, 1);

        // first test the serial... what goes through, what goes not...

        fprintf(stderr, "Finished writing.\n");
        // then test all the commands....
        sleep(10);
        return EXIT_SUCCESS;
    }

    thrd_t rx_thread;
    thrd_create(&rx_thread, cat_rcv, &serial_fd );


    if (tester_mode)
    {

        // test all the commands here...
    }


    //while (running)
    {
        // if (has_some_command...)

        get_frequency(serial_fd, radio_type, &frequency);

        sleep(1);

        set_frequency(serial_fd, radio_type, frequency);

        sleep(1);

        get_frequency(serial_fd, radio_type, &frequency);

//        get_mastercal(serial_fd, radio_type, &frequency);

//        get_bfo(serial_fd, radio_type, &frequency);


        if (ssb_mode != UNDEFINED)
        {
            set_ssb_mode(serial_fd, radio_type, ssb_mode);
            ssb_mode = UNDEFINED;
        }

        sleep(10);

    }

}
