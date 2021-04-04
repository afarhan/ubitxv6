/*
 * Copyright (C) 2020 Rhizomatica <rafael@rhizomatica.org>
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
 * Rhizo-HF-Connector
 *
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <asm/termbits.h>
#include <errno.h>
#include <threads.h>
#include <stdint.h>

#include "serial.h"
#include "ubitx_controller.h"

struct baudrate {
    char       *name;
    int         termios_code;
    int         nonstd_speed;
    int         bootrom_code;
    int         xram_records;
};

int open_serial_port(char *ttyport)
{
    int target_fd = open(ttyport, O_RDWR|O_NONBLOCK);
    if (target_fd < 0)
    {
        fprintf(stderr, "open() serial port error\n");
        perror(ttyport);
        exit(EXIT_FAILURE);
    }

    ioctl(target_fd, TIOCEXCL);
    return target_fd;
}


struct baudrate baud_rate_table[] = {
    /* the first listed rate will be our default */
    {"115200",	B115200,	0,	0,	100},
    {"57600",	B57600,		0,	1,	100},
    {"38400",	B38400,		0,	2,	100},
    {"19200",	B19200,		0,	4,	50},
    /* Non-standard high baud rates */
    {"812500",	BOTHER,		812500,	-1,	1000},
    {"406250",	BOTHER,		406250,	-1,	500},
    {"203125",	BOTHER,		203125,	-1,	250},
    /* table search terminator */
    {NULL,		B0,		0,	-1,	0},
};

struct baudrate *find_baudrate_by_name(char *srch_name)
{
    struct baudrate *br;

    for (br = baud_rate_table; br->name; br++)
        if (!strcmp(br->name, srch_name))
            break;
    if (br->name)
        return(br);
    else
    {
        fprintf(stderr, "error: baud rate \"%s\" not known\n", srch_name);
        return(NULL);
    }
}

struct baudrate *set_serial_baudrate(struct baudrate *br, int target_fd)
{
    struct termios2 target_termios;

    target_termios.c_iflag = IGNBRK;
    target_termios.c_oflag = 0;
    target_termios.c_cflag = br->termios_code | CLOCAL|HUPCL|CREAD|CS8;
    target_termios.c_lflag = 0;
    target_termios.c_cc[VMIN] = 1;
    target_termios.c_cc[VTIME] = 0;
    target_termios.c_ispeed = br->nonstd_speed;
    target_termios.c_ospeed = br->nonstd_speed;
    if (ioctl(target_fd, TCSETSF2, &target_termios) < 0) {
        fprintf(stderr, "ioctl() TCSETSF2 error\n");
        perror("TCSETSF2");
        exit(1);
    }

    return br;
}

void set_fixed_baudrate(char *baudname, int target_fd)
{
    struct baudrate *br;

    br = find_baudrate_by_name(baudname);
    if (!br)
        exit(1); /* error msg already printed */
    set_serial_baudrate(br, target_fd);
}

void key_on(int serial_fd, int radio_type)
{

    if (radio_type == RADIO_TYPE_ICOM)
    {
        int key_on_size = 8;
        uint8_t key_on[8];
        key_on[0] = 0xFE;
        key_on[1] = 0xFE;
        key_on[2] = 0x88;
        key_on[3] = 0xE0;
        key_on[4] = 0x1C;
        key_on[5] = 0x00;
        key_on[6] = 0x01;
        key_on[7] = 0xFD;

        write(serial_fd, key_on, key_on_size);
    }

    if (radio_type == RADIO_TYPE_UBITX)
    {
        int key_on_size = 5;
        uint8_t key_on[5];
        key_on[0] = 0x00;
        key_on[1] = 0x00;
        key_on[2] = 0x00;
        key_on[3] = 0x00;
        key_on[4] = 0x08;
        write(serial_fd, key_on, key_on_size);

    }
}

void key_off(int serial_fd, int radio_type)
{
    if (radio_type == RADIO_TYPE_ICOM)
    {
        int key_off_size = 8;
        uint8_t key_off[8];
        key_off[0] = 0xFE;
        key_off[1] = 0xFE;
        key_off[2] = 0x88;
        key_off[3] = 0xE0;
        key_off[4] = 0x1C;
        key_off[5] = 0x00;
        key_off[6] = 0x00;
        key_off[7] = 0xFD;

        write(serial_fd, key_off, key_off_size);
    }

    if (radio_type == RADIO_TYPE_UBITX)
    {
        int key_off_size = 5;
        uint8_t key_off[5];
        key_off[0] = 0x00;
        key_off[1] = 0x00;
        key_off[2] = 0x00;
        key_off[3] = 0x00;
        key_off[4] = 0x88;
        write(serial_fd, key_off, key_off_size);

    }
}

void set_ssb_mode(int serial_fd, int radio_type, int ssb_mode)
{
    int write_size = 0;
    uint8_t buffer[8];

    if (radio_type == RADIO_TYPE_UBITX)
    {
        if (ssb_mode == LSB)
        {
            printf("LSB\n");
            write_size = 5;
            buffer[0] = 0x00;
            buffer[1] = 0x00;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
            buffer[4] = 0x07;
            write(serial_fd, buffer, write_size);
        }
        if (ssb_mode == USB)
        {
            printf("USB\n");
            write_size = 5;
            buffer[0] = 0x02;
            buffer[1] = 0x00;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
            buffer[4] = 0x07;
            write(serial_fd, buffer, write_size);
       }
    }


}

void get_ssb_mode(int serial_fd, int radio_type, int *ssb_mode)
{
    uint8_t buffer[8];
    int write_size = 5;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0xf0;

    write(serial_fd, buffer, write_size);
}

void set_frequency(int serial_fd, int radio_type, int frequency)
{
    uint8_t buffer[8];
    int write_size = 5;

    memcpy (buffer, &frequency, 4);
    buffer[4] = 0xfa;

    write(serial_fd, buffer, write_size);
}

void get_frequency(int serial_fd, int radio_type, int *frequency)
{
    uint8_t buffer[8];
    int write_size = 5;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0xfb;

    write(serial_fd, buffer, write_size);
}

void get_mastercal(int serial_fd, int radio_type, int *frequency)
{
    uint8_t buffer[8];
    int write_size = 5;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0xff;

    write(serial_fd, buffer, write_size);
}

void get_bfo(int serial_fd, int radio_type, int *frequency)
{
    uint8_t buffer[8];
    int write_size = 5;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0xfe;

    write(serial_fd, buffer, write_size);
}
