/* Rhizo-uuardop: Tools to integrate Ardop to UUCP
 * Copyright (C) 2019 Rhizomatica
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
 * Routines to call uucico when receiving a call
 */

/**
 * @file shm.c
 * @author Rafael Diniz
 * @date 14 Aug 2019
 * @brief Shared memory routines
 *
 * Shared memory routines
 *
 */

#include <sys/ioctl.h>
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
#include <dirent.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <grp.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "shm.h"

bool shm_is_created(key_t key, size_t size)
{
    int shmid = shmget(key, size, 0);

    if (shmid == -1)
    {
        return false;
    }

    return true;
}

// check of key is already not created before calling this!
bool shm_create(key_t key, size_t size)
{
    int shmid = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);

    if (shmid == -1)
    {
        return false;
    }

    return true;
}

bool shm_destroy(key_t key, size_t size)
{
    int shmid = shmget(1, size, 0);

    if (shmid == -1)
    {
        return false;
    }

    shmctl(shmid,IPC_RMID,NULL);

    return true;
}

void *shm_attach(key_t key, size_t size)
{
    int shmid = shmget(key, size, 0);

    if (shmid == -1)
    {
        return NULL;
    }

    return shmat(shmid, NULL, 0);
}

bool shm_dettach(key_t key, size_t size, void *ptr)
{
    int shmid = shmget(key, size, 0);

    if (shmid == -1)
    {
        return false;
    }

    shmdt(ptr);

    return true;
}
