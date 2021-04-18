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

    if (argc < 3)
    {
    manual:
        fprintf(stderr, "Usage modes: \n%s -c command\n", argv[0]);
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

    if (!strcmp(command, "set_frequency"))
    {
        
    }
    else if (!strcmp(command, "get_frequency"))
    {
        
    }


}
