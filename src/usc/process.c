/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface - Client
 *
 * Date: 23-08-2014
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
#include <unistd.h>

#include <panet/panet.h>

#include "debug.h"
#include "config.h"
#include "bitops.h"
#include "mm.h"
#include "conn.h"
#include "runtime.h"
#include "log.h"
#include "print.h"
#include "process.h"
#include "lib.h"

/* Reserved library functions */
static int _process_lib_result_add_run(uint64_t entry_id) {
	if (!(runc.result = mm_realloc(runc.result, sizeof(uint64_t) * (runc.result_nmemb + 1)))) {
		log_warn("_process_lib_result_add_run(): mm_realloc(): %s\n", strerror(errno));
		runc.result_nmemb = 0;
		return -1;
	}

	memcpy(&((uint64_t *) runc.result)[++ runc.result_nmemb], &entry_id, sizeof(uint64_t));

	return 0;
}

static int _process_lib_result_set_stop(uint64_t *entry_list, size_t nmemb) {
	runc.result = entry_list;
	runc.result_nmemb = nmemb;

	return 0;
}

static int _process_lib_result_set_show(struct usched_entry *entry_list, size_t nmemb) {
	runc.result = entry_list;
	runc.result_nmemb = nmemb;

	return 0;
}

int process_client_recv_run(struct usched_entry *entry) {
	int errsv = 0;
	uint64_t entry_id = 0;

	/* Read the response in order to obtain the entry id */
	if (read(runc.fd, &entry_id, sizeof(entry_id)) != sizeof(entry_id)) {
		errsv = errno;
		log_crit("process_client_recv_run(): read() != %zu: %s\n", sizeof(entry_id), strerror(errno));
		errno = errsv;
		return -1;
	}

	entry_id = ntohll(entry_id);

	debug_printf(DEBUG_INFO, "Received Entry ID: 0x%llX\n", entry_id);

	if (bit_test(&runc.flags, USCHED_RUNTIME_FLAG_LIB)) {
		/* This is from library */
		return _process_lib_result_add_run(entry_id);
	} else {
		/* Otherwise print the installed entry */
		print_client_result_run(entry_id);
	}

	return 0;
}

int process_client_recv_stop(struct usched_entry *entry) {
	int i = 0, errsv = 0;
	uint32_t entry_list_nmemb = 0;
	uint64_t *entry_list = NULL;

	/* Read te number of elements to receive */
	if (read(runc.fd, &entry_list_nmemb, sizeof(entry_list_nmemb)) != sizeof(entry_list_nmemb)) {
		errsv = errno;
		log_crit("process_client_recv_stop(): read() != %zu: %s\n", sizeof(entry_list_nmemb), strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Network to Host byte order */
	entry_list_nmemb = ntohl(entry_list_nmemb);

	if (!entry_list_nmemb) {
		log_info("process_client_recv_stop(): No entries were deleted.\n");
		print_client_result_empty();
		return 0;
	}

	/* Alloc sufficient memory to receive the deleted entries list */
	if (!(entry_list = mm_alloc(entry_list_nmemb * sizeof(uint64_t)))) {
		errsv = errno;
		log_crit("process_client_recv_stop(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Receive the deleted entries list */
	if (read(runc.fd, entry_list, entry_list_nmemb * sizeof(uint64_t)) != (entry_list_nmemb * sizeof(uint64_t))) {
		errsv = errno;
		log_crit("process_client_recv_stop(): read() != %zu: %s\n", entry_list_nmemb * sizeof(uint64_t), strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Iterate the received entries list */
	for (i = 0; i < entry_list_nmemb; i ++) {
		/* Network to Host byte order */
		entry_list[i] = ntohll(entry_list[i]);

		debug_printf(DEBUG_INFO, "Entry ID 0x%llX was deleted\n", entry_list[i]);
	}

	if (bit_test(&runc.flags, USCHED_RUNTIME_FLAG_LIB)) {
		/* This is from library */
		return _process_lib_result_set_stop(entry_list, entry_list_nmemb);
	} else {
		/* Otherwise print the deleted entries */
		print_client_result_del(entry_list, entry_list_nmemb);
	}

	/* Free entry_list memory */
	mm_free(entry_list);

	return 0;
}

int process_client_recv_show(struct usched_entry *entry) {
	int i = 0, errsv = 0, ret = -1;
	uint32_t entry_list_nmemb = 0, data_len = 0;
	struct usched_entry *entry_list = NULL;
	size_t p_offset = 0;

	/* Cleanup entry payload if required */
	if (entry->payload) {
		mm_free(entry->payload);
		entry->payload = NULL;
		entry->psize = 0;
	}

	/* Read data size */
	if (read(runc.fd, &data_len, 4) != 4) {
		errsv = errno;
		log_crit("process_client_recv_show(): read(, &data_len, 4) != 4: %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set the entry payload size */
	entry->psize = ntohl(data_len) - 4;

	/* Allocate enough memory to receive the payload */
	if (!(entry->payload = mm_alloc(entry->psize))) {
		errsv = errno;
		log_crit("process_client_recv_show(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Receive the payload */
	if (read(runc.fd, entry->payload, entry->psize) != entry->psize) {
		errsv = errno;
		log_crit("process_client_recv_show(): read(..., entry->payload, entry->psize) != entry->psize: %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt the payload */
	if (entry_payload_decrypt(entry) < 0) {
		errsv = errno;
		log_crit("process_client_recv_show(): entry_payload_decrypt(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set the entry_list_size */
	memcpy(&entry_list_nmemb, entry->payload, sizeof(entry_list_nmemb));

	/* Update payload offset */
	p_offset += sizeof(entry_list_nmemb);

	/* Network to Host byte order */
	entry_list_nmemb = ntohl(entry_list_nmemb);

	if (!entry_list_nmemb) {
		log_info("process_client_recv_show(): No entries were found.\n");
		print_client_result_empty();
		return 0;
	}

	/* Alloc the entries array */
	if (!(entry_list = mm_alloc(entry_list_nmemb * sizeof(struct usched_entry)))) {
		errsv = errno;
		log_crit("process_client_recv_show(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset array memory */
	memset(entry_list, 0, entry_list_nmemb * sizeof(struct usched_entry));

	/* Receive the entries */
	for (i = 0; i < entry_list_nmemb; i ++) {
		/* Read the next entry */
		memcpy(&entry_list[i], entry->payload + p_offset, offsetof(struct usched_entry, psize));
		p_offset += offsetof(struct usched_entry, psize);

		/* Convert Network to Host byte order */
		entry_list[i].id = ntohll(entry_list[i].id);
		entry_list[i].flags = ntohl(entry_list[i].flags);
		entry_list[i].uid = ntohl(entry_list[i].uid);
		entry_list[i].gid = ntohl(entry_list[i].gid);
		entry_list[i].trigger = ntohl(entry_list[i].trigger);
		entry_list[i].step = ntohl(entry_list[i].step);
		entry_list[i].expire = ntohl(entry_list[i].expire);

		/* Read the entry username */
		memcpy(entry_list[i].username, entry->payload + p_offset, CONFIG_USCHED_AUTH_USERNAME_MAX);
		p_offset += CONFIG_USCHED_AUTH_USERNAME_MAX;

		/* Read the subject size */
		memcpy(&entry_list[i].subj_size, entry->payload + p_offset, 4);
		p_offset += 4;

		/* Convert Network to Host byte order */
		entry_list[i].subj_size = ntohl(entry_list[i].subj_size);

		/* Allocate the subject memory */
		if (!(entry_list[i].subj = mm_alloc(entry_list[i].subj_size + 1))) {
			errsv = errno;
			log_crit("process_client_recv_show(): mm_alloc(%d): %s\n", entry_list[i].subj_size + 1, strerror(errno));
			goto _recv_show_finish;
		}

		/* Reset subject memory */
		memset(entry_list[i].subj, 0, entry_list[i].subj_size + 1);

		/* Read the subject contents */
		memcpy(entry_list[i].subj, entry->payload + p_offset, entry_list[i].subj_size + 1);
		p_offset += entry_list[i].subj_size + 1;
	}

	/* Check if this is a library call */
	if (bit_test(&runc.flags, USCHED_RUNTIME_FLAG_LIB)) {
		/* This is from library */
		return _process_lib_result_set_show(entry_list, entry_list_nmemb);
	} else {
		/* Otherwise print the received entries */
		print_client_result_show(entry_list, entry_list_nmemb);
	}

	ret = 0;
	i --;

_recv_show_finish:
	for (; i >= 0; i --) {
		if (entry_list[i].subj)
			mm_free(entry_list[i].subj);
	}

	mm_free(entry_list);

	errno = errsv;

	return ret;
}

