/**
 * @file daemon.c
 * @brief uSched
 *        Daemon Main Component
 *
 * Date: 24-06-2014
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
#include <stdlib.h>

#include "runtime.h"
#include "log.h"
#include "daemonize.h"
#include "conn.h"

static void _init(int argc, char **argv) {
	if (runtime_daemon_init(argc, argv) < 0) {
		log_crit("_init(): runtime_daemon_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (daemonize() < 0) {
		log_crit("_init(): daemonize(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_daemon_destroy();
}

static void _loop(void) {
	/* Process connections */
	conn_daemon_process();
}

int main(int argc, char *argv[]) {
	_init(argc, argv);

	_loop();

	_destroy();

	return 0;
}

