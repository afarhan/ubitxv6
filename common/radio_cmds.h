/* Rhizomatica transceiver (Kurupira) serial commands
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

// we need a 8bit clean serial connection
// PTT commands need priority
// We don't care about endianess (little endian assumed)
// IT WILL NOT WORK ON BIG ENDIAN MACHINES RIGHT NOW!

// the radio commands
#define CMD_PTT_ON 0x08
#define CMD_PTT_OFF 0x88

#define CMD_GET_FREQ 0xfb
#define CMD_SET_FREQ 0xfc

#define CMD_GET_MODE 0xf0
#define CMD_SET_MODE 0x07

#define CMD_GET_TXRX_STATUS 0xf1

#define CMD_GET_PROTECTION_STATUS 0xf2

#define CMD_GET_MASTERCAL 0xee
#define CMD_SET_MASTERCAL 0xfd

#define CMD_GET_BFO 0xef
#define CMD_SET_BFO 0xec

#define CMD_GET_FWD 0xf3
#define CMD_GET_REF 0xf4

#define CMD_GET_LED_STATUS 0xf6
#define CMD_SET_LED_STATUS 0xf5

#define CMD_GET_BYPASS_STATUS 0xf8
#define CMD_SET_BYPASS_STATUS 0xf9

// radio responses type
#define CMD_RESP_NOTHING 0
#define CMD_RESP_SHORT 1
#define CMD_RESP_LONG 2

// radio responses
// 5 bytes responses <= 0x30
#define CMD_RESP_GET_FREQ_ACK 0x01
#define CMD_RESP_GET_MASTERCAL_ACK 0x02
#define CMD_RESP_GET_BFO_ACK 0x03

#define CMD_RESP_GET_FWD_ACK 0x04
#define CMD_RESP_GET_REF_ACK 0x05

#define CMD_LAST_5BYTES 0x30


// 1 byte responses, >= 0xd0
#define CMD_RESP_PTT_ON_ACK 0xf1
#define CMD_RESP_PTT_ON_NACK 0xf2

#define CMD_RESP_PTT_OFF_ACK 0xf3
#define CMD_RESP_PTT_OFF_NACK 0xf4

#define CMD_RESP_SET_FREQ_ACK 0xf0

#define CMD_RESP_SET_MODE_ACK 0xf5

#define CMD_RESP_GET_MODE_USB 0xf6
#define CMD_RESP_GET_MODE_LSB 0xf7

#define CMD_RESP_GET_TXRX_INTX 0xf8
#define CMD_RESP_GET_TXRX_INRX 0xf9

#define CMD_RESP_GET_PROTECTION_ON 0xfa
#define CMD_RESP_GET_PROTECTION_OFF 0xfb

#define CMD_RESP_SET_MASTERCAL_ACK 0xfc

#define CMD_RESP_SET_BFO_ACK 0xfd

#define CMD_RESP_GET_LED_STATUS_ON 0xe0
#define CMD_RESP_GET_LED_STATUS_OFF 0xe1

#define CMD_RESP_SET_LED_STATUS_ACK 0xe2

#define CMD_RESP_GET_BYPASS_STATUS_ON 0xe3
#define CMD_RESP_GET_BYPASS_STATUS_OFF 0xe4

#define CMD_RESP_SET_BYPASS_STATUS_ACK 0xe5

#define CMD_RESP_WRONG_COMMAND 0xe6

// autonomous commands from the radio
#define CMD_ALERT_PROTECTION_ON 0xe7
