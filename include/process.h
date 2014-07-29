/**
 * @file process.h
 * @brief uSched
 *        Data Processing interface header
 *
 * Date: 11-07-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usched is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usched.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef USCHED_PROCESS_H
#define USCHED_PROCESS_H

#include <rtsaio/rtsaio.h>

/* Prototypes */
struct usched_entry *process_daemon_recv_create(struct async_op *aop);
int process_daemon_recv_update(struct async_op *aop, struct usched_entry *entry);
int process_client_recv_run(void);
int process_client_recv_stop(void);
int process_client_recv_show(void);

#endif

