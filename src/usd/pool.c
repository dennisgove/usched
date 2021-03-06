/**
 * @file pool.c
 * @brief uSched
 *        Pool handlers interface
 *
 * Date: 08-03-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
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
#include <pthread.h>

#include <pall/cll.h>

#include "runtime.h"
#include "pool.h"
#include "entry.h"
#include "log.h"

int pool_daemon_init(void) {
	int errsv = 0;

	/* Initialize active scheduling entries pool */
	if (!(rund.apool = pall_cll_init(&entry_compare, &entry_destroy, &entry_daemon_serialize, &entry_daemon_unserialize))) {
		errsv = errno;
		log_crit("pool_daemon_init(): rund.apool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	(void) rund.apool->set_config(rund.apool, (ui32_t) (CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD));

	/* Initialize connection pool */
	if (!(rund.rpool = pall_cll_init(&entry_compare, &entry_destroy, &entry_daemon_serialize, &entry_daemon_unserialize))) {
		errsv = errno;
		log_crit("pool_daemon_init(): rund.rpool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	(void) rund.rpool->set_config(rund.rpool, (ui32_t) (CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD));

	return 0;
}

void pool_daemon_destroy(void) {
	/* TODO: Grant that conn_daemon_destroy() and schedule_daemon_destroy() were already called
	 * before stepping ahead this point.
	 */

	/* NOTE: Any notification threads and scheduled routines are disabled at this point. This
	 * is granted by libpsched and librtsaio. Assuming that the libraries are doing it right and
	 * conn_daemon_destroy() and schedule_daemon_destroy() functions are called before this one,
	 * there's no risk of destroying the active pool.
	 */

	pthread_mutex_lock(&rund.mutex_apool);
	if (rund.apool) {
		pall_cll_destroy(rund.apool);
		rund.apool = NULL;
	}
	pthread_mutex_unlock(&rund.mutex_apool);

	/* NOTE: conn_daemon_destroy() must have been called at this time in order to ensure that
	 * the destruction of remote connections pool won't cause any invalid memory accesses.
	 */
	pthread_mutex_lock(&rund.mutex_rpool);
	if (rund.rpool) {
		pall_cll_destroy(rund.rpool);
		rund.rpool = NULL;
	}
	pthread_mutex_unlock(&rund.mutex_rpool);
}

