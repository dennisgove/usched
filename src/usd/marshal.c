/**
 * @file marshal.c
 * @brief uSched
 *        Serialization / Unserialization interface
 *
 * Date: 27-01-2015
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
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <psched/sched.h>

#include <fsop/file.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "marshal.h"
#include "log.h"
#include "entry.h"
#include "schedule.h"

int marshal_daemon_init(void) {
	int errsv = 0;

	if ((rund.ser_fd = open(rund.config.core.file_serialize, O_CREAT | O_SYNC | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): open(\"%s\", ...): %s\n", rund.config.core.file_serialize, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Test lock on serialization file */
	if (lockf(rund.ser_fd, F_TLOCK, 0) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): lockf(): Serialization file is locked by another process.\n");
		errno = errsv;
		return -1;
	}

	/* Acquire lock on serialization file */
	if (lockf(rund.ser_fd, F_LOCK, 0) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): lockf(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int marshal_daemon_serialize_pools(void) {
	int errsv = 0;
	int ret = -1;

	pthread_mutex_lock(&rund.mutex_apool);

#if CONFIG_SERIALIZE_ON_REQ == 1
	if (lseek(rund.ser_fd, 0, SEEK_SET) == (off_t) -1) {
		errsv = errno;
		pthread_mutex_unlock(&rund.mutex_apool);
		log_warn("marshal_daemon_serialize_pools(): lseek(%d, 0, SEEK_SET): %s\n", rund.ser_fd, strerror(errno));
		errno = errsv;
		return -1;
	}
#endif

	/* Serialize the active pool */
	if ((ret = rund.apool->serialize(rund.apool, rund.ser_fd)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_serialize_pools(): rund.apool->serialize(): %s\n", strerror(errno));
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	errno = errsv;

	return ret;
}

int marshal_daemon_unserialize_pools(void) {
	int ret = -1, errsv = errno;
	struct usched_entry *entry = NULL;

	pthread_mutex_lock(&rund.mutex_apool);

	/* Unserialize active pool */
	if ((ret = rund.apool->unserialize(rund.apool, rund.ser_fd)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_unserialize_pools(): rund.apool->unserialize(): %s\n", strerror(errno));
		goto _unserialize_finish;
	}

	/* Activate all the unserialized entries through libpsched */
	for (rund.apool->rewind(rund.apool, 0); (entry = rund.apool->iterate(rund.apool)); ) {
		/* Update the trigger value based on step and current time */
		while (entry->step && (entry->trigger <= time(NULL))) {
			/* Check if we've to align (month or year?) and update trigger accordingly */
			if (entry_has_flag(entry, USCHED_ENTRY_FLAG_MONTHDAY_ALIGN)) {
				entry->trigger += schedule_step_ts_add_month(entry->trigger, entry->step / 2592000);
			} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_YEARDAY_ALIGN)) {
				entry->trigger += schedule_step_ts_add_year(entry->trigger, entry->step / 31536000);
			} else {
				/* No alignment required */
				entry->trigger += entry->step;
			}
		}

		/* Check if the trigger remains valid, i.e., does not exceed the expiration time */
		if (entry->expire && (entry->trigger >= entry->expire)) {
			log_info("marshal_daemon_unserialize_pools(): An entry is expired (ID: 0x%llX).\n", entry->id);

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);
			continue;
		}

		/* If the trigger time is lesser than current time and no step is defined, invalidate this entry. */
		if ((entry->trigger <= time(NULL)) && !entry->step) {
			log_info("marshal_daemon_unserialize_pools(): Found an invalid entry (ID: 0x%llX).\n", entry->id);

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);
			continue;
		}

		debug_printf(DEBUG_INFO, "[TIME: %lu]: entry->id: 0x%016llX, entry->trigger: %lu, entry->step: %lu, entry->expire: %lu\n", time(NULL), entry->id, entry->trigger, entry->step, entry->expire);

		/* Install a new scheduling entry based on the current entry parameters */
		if ((entry->reserved.psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_daemon_pmq_dispatch, entry)) == (pschedid_t) -1) {
			/* TODO or FIXME: This is critical, the entry will be lost and we can't force
			 * a gracefully daemon restart or the serialization data will be overwritten
			 * with a missing entry... Something must be done here to prevent such damage
			 */
			log_warn("marshal_daemon_unserialize_pools(): psched_timestamp_arm(): %s\n", strerror(errno));

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);

			continue;
		}
	}

_unserialize_finish:
	pthread_mutex_unlock(&rund.mutex_apool);

	errno = errsv;

	return ret;
}

int marshal_daemon_backup(void) {
	int errsv = 0;
	size_t len = strlen(rund.config.core.file_serialize) + 50;
	char *file_bak = NULL;

	if (!(file_bak = mm_alloc(len))) {
		errsv = errno;
		log_warn("marshal_daemon_backup(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(file_bak, 0, len);

	snprintf(file_bak, len - 1, "%s-%lu-%u", rund.config.core.file_serialize, time(NULL), getpid());

	if (fsop_cp(rund.config.core.file_serialize, file_bak, 8192) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_backup(): fsop_cp(): %s\n", strerror(errno));
		mm_free(file_bak);
		errno = errsv;
		return -1;
	}

	mm_free(file_bak);

	return 0;
}

void marshal_daemon_wipe(void) {
	if (unlink(rund.config.core.file_serialize) < 0)
		log_warn("marshal_daemon_wipe(): unlink(\"%s\"): %s\n", rund.config.core.file_serialize, strerror(errno));
}

void marshal_daemon_destroy(void) {
#if CONFIG_USE_SYNCFS == 1
	if (syncfs(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): syncfs(): %s\n", strerror(errno));
#else
	sync();
#endif
	/* Remove lock from serialization file */
	if (lockf(rund.ser_fd, F_ULOCK, 0) < 0)
		log_warn("marshal_daemon_init(): lockf(): %s\n", strerror(errno));

	if (close(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): close(): %s\n", strerror(errno));
}

