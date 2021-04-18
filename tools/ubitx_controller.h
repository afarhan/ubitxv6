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

#include <stdatomic.h>

#define SYSV_SHM_KEY_STR 66650 // key for the controller_conn struct


typedef struct{

    char service_command[5];
    pthread_mutex_t ptt_mutex;
    pthread_cond_t ptt_condition;


    char response_service[5];
    atomic_bool response_service_type;
    atomic_bool response_available;

    // special response for ptt
    atomic_char ptt_last_response;

    // special response for ptt
    atomic_bool protection_alert;


    int radio_fd;

    // no guard
    // atomic_bool connected_service;
    // atomic_bool connected_ptt;

} controller_conn;


#endif // HAVE_UBITXCONTROLLER_H__
