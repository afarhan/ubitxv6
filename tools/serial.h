/* Rhizo-uuhf: Tools to integrate HF TNCs to UUCP
 * Copyright (C) 2020-2021 Rhizomatica
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

#ifndef HAVE_SERIAL_H__
#define HAVE_SERIAL_H__

#include <stdbool.h>

#define MAX_MODEM_PATH 4096
#define MAX_BUF_SIZE 4096

#define RADIO_TYPE_ICOM 0
#define RADIO_TYPE_UBITX 1

void key_on(int serial_fd, int radio_type);
void key_off(int serial_fd, int radio_type);

int open_serial_port(char *ttyport);
void set_fixed_baudrate(char *baudname, int target_fd);

void set_ssb_mode(int serial_fd, int radio_type, int ssb_mode);
void get_ssb_mode(int serial_fd, int radio_type, int *ssb_mode);

void set_frequency(int serial_fd, int radio_type, int frequency);
void get_frequency(int serial_fd, int radio_type, int *frequency);

void get_mastercal(int serial_fd, int radio_type, int *frequency);

void get_bfo(int serial_fd, int radio_type, int *frequency);

#endif // HAVE_SERIAL_H__
