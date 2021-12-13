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

#ifndef HAVE_UBITXCONTROLLER_H__
#define HAVE_UBITXCONTROLLER_H__

#include <stdint.h>
#include <stdatomic.h>

#define SYSV_SHM_KEY_STR 66650 // key for the controller_conn struct


typedef struct{

    uint8_t service_command[5];
    pthread_mutex_t cmd_mutex;
    pthread_cond_t cmd_condition;

    pthread_mutex_t response_mutex;

    uint8_t response_service[5];
    atomic_bool response_service_type;
    atomic_bool response_available;

    // special response for ptt
    atomic_uchar ptt_last_response;

    // Protection alert ON!
    atomic_bool protection_alert;

    int radio_fd;

} controller_conn;


#endif // HAVE_UBITXCONTROLLER_H__
