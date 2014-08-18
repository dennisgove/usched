/**
 * @file category.c
 * @brief uSched
 *        Category processing interface
 *
 * Date: 18-08-2014
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
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>

#include "log.h"
#include "users.h"
#include "usage.h"
#include "print.h"
#include "input.h"

int category_users_add(size_t argc, char **args) {
	int errsv = 0;
	char *endptr = NULL;
	char *username = NULL;
	char *password = NULL;
	char pw_input[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	char pw_input_repeat[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	uid_t uid = (uid_t) -1;
	gid_t gid = (gid_t) -1;

	/* Usage: <username> <uid> <gid> <password> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "add users");
		log_warn("category_users_add(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc > 4) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "change users");
		log_warn("category_users_add(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	} if (argc == 3) {
		printf("Password: ");

		if (input_password(pw_input, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_add(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		printf("\nRepeat password: ");

		if (input_password(pw_input_repeat, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_add(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		if (strcmp(pw_input, pw_input_repeat)) {
			puts("\nPasswords do not match.\n");
			log_warn("category_users_add(): Passwords do not match.\n");
			errno = EINVAL;
			return -1;
		}

		puts("");

		if (strlen(password) < CONFIG_USCHED_AUTH_PASSWORD_MIN) {
			puts("Password is too short (it must be at least 8 characters long).\n");
			log_warn("category_users_add(): Password is too short (it must be at least 8 characters long).\n");
			errno = EINVAL;
			return -1;
		}

		password = pw_input;
	} else {
		password = args[3];
	}

	username = args[0];

	uid = strtoul(args[1], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid UID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	gid = strtoul(args[2], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid GID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	/* Add the user */
	if (users_admin_config_add(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("category_users_add(): users_admin_config_add(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_added(username);

	return 0;
}

int category_users_delete(size_t argc, char **args) {
	int errsv = 0;
	char *username = NULL;

	/* Usage: <username> */
	if (argc != 1) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "delete users");
		log_warn("category_users_delete(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	username = args[0];

	/* Delete the user */
	if (users_admin_config_delete(username) < 0) {
		errsv = errno;
		log_warn("category_users_delete(): users_admin_config_delete(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_deleted(username);

	return 0;
}

int category_users_change(size_t argc, char **args) {
	int errsv = 0;
	char *endptr = NULL;
	char *username = NULL;
	char *password = NULL;
	char pw_input[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	char pw_input_repeat[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	uid_t uid = (uid_t) -1;
	gid_t gid = (gid_t) -1;

	memset(pw_input, 0, sizeof(pw_input));

	/* Usage: <username> <uid> <gid> <password> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "change users");
		log_warn("category_users_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc > 4) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "change users");
		log_warn("category_users_change(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc == 3) {
		printf("Password: ");

		if (input_password(pw_input, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_change(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		printf("\nRepeat password: ");

		if (input_password(pw_input_repeat, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_change(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		if (strcmp(pw_input, pw_input_repeat)) {
			puts("\nPasswords do not match.\n");
			log_warn("category_users_change(): Passwords do not match.\n");
			errno = EINVAL;
			return -1;
		}

		puts("");

		password = pw_input;
	} else {
		password = args[3];
	}

	username = args[0];

	uid = strtoul(args[1], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_change(): Invalid UID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	gid = strtoul(args[2], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_change(): Invalid GID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	/* Change the user */
	if (users_admin_config_change(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("category_users_change(): users_admin_config_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_changed(username);

	return 0;
}

int category_users_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: no arguments */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "show users");
		log_warn("category_users_change(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	/* Show the users */
	if (users_admin_config_show() < 0) {
		errsv = errno;
		log_warn("category_users_show(): users_admin_config_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

