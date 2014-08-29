/**
 * @file gc.c
 * @brief uSched
 *        Garbage Collector interface
 *
 * Date: 29-08-2014
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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pall/lifo.h>

#include "mm.h"
#include "log.h"

/* TODO: Cleanup procedure isn't yet being called. */

static struct lifo_handler *_gc = NULL;

void _gc_data_destroy(void *data) {
	mm_free(data);
}

int gc_insert(void *data) {
	return _gc->push(_gc, data);
}

void gc_cleanup(void) {
	void *data = NULL;

	while ((data = _gc->pop(_gc)))
		_gc_data_destroy(data);
}

int gc_init(void) {
	int errsv = 0;

	if (!(_gc = pall_lifo_init(&_gc_data_destroy, NULL, NULL))) {
		errsv = errno;
		log_warn("gc_init(): pall_lifo_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void gc_destroy(void) {
	pall_lifo_destroy(_gc);
}

