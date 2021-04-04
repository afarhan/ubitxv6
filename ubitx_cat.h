/* Rhizomatica Firmware for uBitx based Kurupira-1
 *
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

#pragma once

// to cope with wrong sized commands / errors
#define CAT_RECEIVE_TIMEOUT 500

#define CAT_MODE_LSB            0x00
#define CAT_MODE_USB            0x01

#define ACK 0 // 1 byte response
#define LONG_ACK 1 // 5 bytes response
#define NACK 1

#define CMD_PTT_ON 0x08
#define CMD_PTT_OFF 0x88
#define CMD_SET_SSB_MODE 0x07
#define CMD_GET_SSB_MODE 0xf0
#define CMD_GET_TXRX_STATUS 0xf1
#define CMD_GET_PROTECTION_STATUS 0xf2
// ...

#define CMD_RESP_PTT_ON_ACK  0x01
#define CMD_RESP_PTT_ON_NACK 0x02
#define CMD_RESP_SET_SSB_MODE_ACK 0x03
#define CMD_RESP_GET_SSB_MODE_ACK 0x04
// ... 

void checkCAT();
