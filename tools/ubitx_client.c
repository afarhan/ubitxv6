/* ubitx_client
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
#include <unistd.h>

#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <threads.h>
#include <pthread.h>

#include "serial.h"
#include "ubitx_controller.h"
#include "../common/radio_cmds.h"

#include "shm.h"

int main(int argc, char *argv[])
{
    controller_conn *connector = NULL;
    char command[64];
    char command_argument[64];
    bool is_ptt = false;

    if (argc < 3)
    {
    manual:
        fprintf(stderr, "Usage modes: \n%s -c command [-a command_argument]\n", argv[0]);
        fprintf(stderr, "%s -h\n", argv[0]);
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, " -c command                 Runs the specified command\n");
        fprintf(stderr, " -a command_argument        Runs the specified command\n");
        fprintf(stderr, " -h                         Prints this help.\n");
        exit(EXIT_FAILURE);
    }

    if (shm_is_created(SYSV_SHM_KEY_STR, sizeof(controller_conn)) == false)
    {
        fprintf(stderr, "Connector SHM not created. Is uuardopd running?\n");
        return EXIT_FAILURE;
    }
    connector = shm_attach(SYSV_SHM_KEY_STR, sizeof(controller_conn));

    int opt;
    while ((opt = getopt(argc, argv, "hc:a:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            goto manual;
            break;
        case 'c':
            strcpy(command, optarg);
            break;
        case 'a':
            strcpy(command_argument, optarg);
            break;
        default:
            goto manual;
        }
    }

    pthread_mutex_lock(&connector->ptt_mutex);

    // we clear any previous response not properly read...
    if (connector->response_available == 1)
    {
        fprintf(stderr, "Previous queue response not read!\n");
        connector->response_available = 0;
    }

    if (!strcmp(command, "ptt_on"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_PTT_ON;
        is_ptt = true;
    }
    else if (!strcmp(command, "ptt_off"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_PTT_OFF;
        is_ptt = true;
    }
    else if (!strcmp(command, "get_frequency"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_FREQ;
    }
    else if (!strcmp(command, "set_frequency"))
    {
        uint32_t freq = (uint32_t) atoi(command_argument);
        memcpy(connector->service_command, &freq, 4);
        connector->service_command[4] = CMD_SET_FREQ;
    }
    else if (!strcmp(command, "get_mode"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_MODE;
    }
    else if (!strcmp(command, "set_mode"))
    {
        if (!strcmp(command_argument, "lsb") || !strcmp(command_argument, "LSB"))
            connector->service_command[0] = 0x00;

        if (!strcmp(command_argument, "usb") || !strcmp(command_argument, "USB"))
            connector->service_command[0] = 0x01;

        connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_SET_MODE;
    }
    else if (!strcmp(command, "get_txrx_status"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_TXRX_STATUS;
    }
    else if (!strcmp(command, "get_protection_status"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_PROTECTION_STATUS;
    }
    else if (!strcmp(command, "get_mastercal"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_MASTERCAL;
    }
    else if (!strcmp(command, "set_mastercal"))
    {
        uint32_t freq = (uint32_t) atoi(command_argument);
        memcpy(connector->service_command, &freq, 4);
        connector->service_command[4] = CMD_SET_MASTERCAL;
    }
    else if (!strcmp(command, "get_bfo"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_BFO;
    }
    else if (!strcmp(command, "set_bfo"))
    {
        uint32_t freq = (uint32_t) atoi(command_argument);
        memcpy(connector->service_command, &freq, 4);
        connector->service_command[4] = CMD_SET_BFO;
    }
    else if (!strcmp(command, "get_fwd"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_FWD;
    }
    else if (!strcmp(command, "get_ref"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_REF;
    }
    else if (!strcmp(command, "get_led_status"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_LED_STATUS;
    }
    else if (!strcmp(command, "set_led_status"))
    {
        if (!strcmp(command_argument, "1") || !strcmp(command_argument, "true") || !strcmp(command_argument, "on") || !strcmp(command_argument, "ON"))
            connector->service_command[0] = 0x01;

        if (!strcmp(command_argument, "0") || !strcmp(command_argument, "false") || !strcmp(command_argument, "off") || !strcmp(command_argument, "OFF"))
            connector->service_command[0] = 0x00;

        connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_SET_LED_STATUS;
    }
    else if (!strcmp(command, "get_bypass_status"))
    {
        connector->service_command[0] = connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_GET_BYPASS_STATUS;
    }
    else if (!strcmp(command, "set_bypass_status"))
    {
        if (!strcmp(command_argument, "1") || !strcmp(command_argument, "true") || !strcmp(command_argument, "on") || !strcmp(command_argument, "ON"))
            connector->service_command[0] = 0x01;

        if (!strcmp(command_argument, "0") || !strcmp(command_argument, "false") || !strcmp(command_argument, "off") || !strcmp(command_argument, "OFF"))
            connector->service_command[0] = 0x00;

        connector->service_command[1] = connector->service_command[2] = connector->service_command[3] = 0x00;
        connector->service_command[4] = CMD_SET_BYPASS_STATUS;
    }

    pthread_cond_signal(&connector->ptt_condition);
    pthread_mutex_unlock(&connector->ptt_mutex);

    if (is_ptt)
    {
        switch (connector->ptt_last_response)
        {
        case CMD_RESP_PTT_ON_ACK:
        case CMD_RESP_PTT_OFF_ACK:
            printf("OK\n");
            break;
        case CMD_RESP_PTT_ON_NACK:
        case CMD_RESP_PTT_OFF_NACK:
            printf("NOK\n");
            break;
        default:
            printf("ERROR\n");
        }
        return EXIT_SUCCESS;
    }


    int tries = 0;
    while (connector->response_available == 0 && tries < 20)
    {
        usleep(50000); // 50 ms
        tries++;
    }

    if (connector->response_available == 0)
        printf("ERROR\n");
    else
    {
        uint32_t freq;
        uint16_t measure;

        switch(connector->response_service[0])
        {
        case CMD_RESP_SET_FREQ_ACK:
        case CMD_RESP_SET_MODE_ACK:
        case CMD_RESP_SET_MASTERCAL_ACK:
        case CMD_RESP_SET_BFO_ACK:
        case CMD_RESP_SET_LED_STATUS_ACK:
        case CMD_RESP_SET_BYPASS_STATUS_ACK:
            printf("OK\n");
            break;
            // continue here...
        case CMD_RESP_GET_MODE_USB:
            printf("USB\n");
            break;
        case CMD_RESP_GET_MODE_LSB:
            printf("LSB\n");
            break;
        case CMD_RESP_GET_TXRX_INTX:
            printf("INTX\n");
            break;
        case CMD_RESP_GET_TXRX_INRX:
            printf("INRX\n");
            break;
        case CMD_RESP_GET_LED_STATUS_OFF:
            printf("LED_OFF\n");
            break;
        case CMD_RESP_GET_LED_STATUS_ON:
            printf("LED_ON\n");
            break;
        case CMD_RESP_GET_PROTECTION_ON:
            printf("PROTECTION_ON\n");
            break;
        case CMD_RESP_GET_PROTECTION_OFF:
            printf("PROTECTION_OFF\n");
            break;
        case CMD_RESP_GET_BYPASS_STATUS_ON:
            printf("BYPASS_ON\n");
            break;
        case CMD_RESP_GET_BYPASS_STATUS_OFF:
            printf("BYPASS_OFF\n");
            break;
        case CMD_RESP_GET_FREQ_ACK:
            memcpy (&freq, connector->response_service+1, 4);
            printf("%u\n", freq);
            break;
        case CMD_RESP_GET_MASTERCAL_ACK:
            memcpy (&freq, connector->response_service+1, 4);
            printf("%u\n", freq);
            break;
        case CMD_RESP_GET_BFO_ACK:
            memcpy (&freq, connector->response_service+1, 4);
            printf("%u\n", freq);
            break;
        case CMD_RESP_GET_FWD_ACK:
            memcpy (&measure, connector->response_service+1, 2);
            printf("%hu\n", measure);
            break;
        case CMD_RESP_GET_REF_ACK:
            memcpy (&measure, connector->response_service+1, 2);
            printf("%hu\n", measure);
            break;
        case CMD_RESP_WRONG_COMMAND:
        default:
            printf("ERROR\n");
        }

        connector->response_available = 0;

    }

}
