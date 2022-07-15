/* Rhizomatica HERMES transceiver serial commands
 * Copyright (C) 2021-2022 Rhizomatica
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

// we need a 8bit clean serial connection
// PTT commands have priority
// We don't care about endianess (little endian assumed)
// IT MIGHT NOT WORK ON BIG ENDIAN MACHINES RIGHT NOW!


// radio responses type
#define CMD_RESP_SHORT 1
#define CMD_RESP_LONG 2


// the radio commands - don't change this for compatibility purposes!
#define CMD_PTT_ON 0x08
#define CMD_PTT_OFF 0x88

#define CMD_RESET_PROTECTION 0x90

#define CMD_GET_FREQ 0x91
#define CMD_SET_FREQ 0x92

#define CMD_GET_MODE 0x93
#define CMD_SET_MODE 0x94

#define CMD_GET_TXRX_STATUS 0x95

#define CMD_GET_PROTECTION_STATUS 0x96

#define CMD_GET_MASTERCAL 0x97
#define CMD_SET_MASTERCAL 0x98

#define CMD_GET_BFO 0x99
#define CMD_SET_BFO 0x9a

#define CMD_GET_FWD 0x9b
#define CMD_GET_REF 0x9c

#define CMD_GET_LED_STATUS 0x9d
#define CMD_SET_LED_STATUS 0x9e

#define CMD_GET_BYPASS_STATUS 0x9f
#define CMD_SET_BYPASS_STATUS 0xa0

#define CMD_SET_SERIAL 0xa1
#define CMD_GET_SERIAL 0xa2

#define CMD_SET_REF_THRESHOLD 0xe0
#define CMD_GET_REF_THRESHOLD 0xe1

#define CMD_SET_RADIO_DEFAULTS 0xea
#define CMD_RESTORE_RADIO_DEFAULTS 0xeb

#define CMD_RADIO_RESET 0xef

#define CMD_GPS_CALIBRATE 0xd0

// radio responses
// 5 bytes responses
#define CMD_RESP_GET_FREQ_ACK 0xa3
#define CMD_RESP_GET_MASTERCAL_ACK 0xa4
#define CMD_RESP_GET_BFO_ACK 0xa5

#define CMD_RESP_GET_FWD_ACK 0xa6
#define CMD_RESP_GET_REF_ACK 0xa7

#define CMD_RESP_GET_SERIAL_ACK 0xa8

#define CMD_RESP_GET_REF_THRESHOLD_ACK 0xe2

// 1 byte responses
#define CMD_RESP_PTT_ON_ACK 0xa9
#define CMD_RESP_PTT_ON_NACK 0xaa

#define CMD_RESP_PTT_OFF_ACK 0xab
#define CMD_RESP_PTT_OFF_NACK 0xac

#define CMD_RESP_SET_FREQ_ACK 0xad

#define CMD_RESP_SET_MODE_ACK 0xaf

#define CMD_RESP_GET_MODE_USB 0xb0
#define CMD_RESP_GET_MODE_LSB 0xb1

#define CMD_RESP_GET_TXRX_INTX 0xb2
#define CMD_RESP_GET_TXRX_INRX 0xb3

#define CMD_RESP_GET_PROTECTION_ON 0xb4
#define CMD_RESP_GET_PROTECTION_OFF 0xb5

#define CMD_RESP_SET_MASTERCAL_ACK 0xb6

#define CMD_RESP_SET_BFO_ACK 0xb7

#define CMD_RESP_GET_LED_STATUS_ON 0xb8
#define CMD_RESP_GET_LED_STATUS_OFF 0xb9

#define CMD_RESP_SET_LED_STATUS_ACK 0xba

#define CMD_RESP_GET_BYPASS_STATUS_ON 0xbb
#define CMD_RESP_GET_BYPASS_STATUS_OFF 0xbc

#define CMD_RESP_SET_BYPASS_STATUS_ACK 0xbd

#define CMD_RESP_RESET_PROTECTION_ACK 0xbe

#define CMD_RESP_SET_SERIAL_ACK 0xbf

#define CMD_RESP_SET_REF_THRESHOLD_ACK 0xe3

#define CMD_RESP_SET_RADIO_DEFAULTS_ACK 0xec

#define CMD_RESP_RESTORE_RADIO_DEFAULTS_ACK 0xed

#define CMD_RESP_GPS_CALIBRATE_ACK 0xd1

#define CMD_RESP_WRONG_COMMAND 0xc0

// used for PTT responses
#define CMD_ALERT_PROTECTION_ON 0xe7
