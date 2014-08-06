/**
 * @file category.h
 * @brief uSched
 *       Category processing interface header
 *
 * Date: 06-08-2014
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

#ifndef USCHED_CATEGORY_H
#define USCHED_CATEGORY_H

#include <stdio.h>

/* Prototypes */
int category_users_add(size_t argc, char **args);
int category_users_delete(size_t argc, char **args);
int category_users_change(size_t argc, char **args);
int category_users_show(size_t argc, char **args);

#endif