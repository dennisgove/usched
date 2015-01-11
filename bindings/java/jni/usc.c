/**
 * @file usc.c
 * @brief uSched JNI Interface
 *        uSched JNI interface - Client
 *
 * Date: 11-01-2015
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
#include <stdlib.h>
#include <jni.h>

#include "JNIUsc.h"

#include <usched/lib.h>

JNIEXPORT jstring JNICALL Java_JNIUsc_nativeUsc(JNIEnv *env, jobject obj) {
	char *req;
	jstring ret = NULL;

	if (!(req = malloc(32)))
		return ret;

	strcpy(req, "Testing uSched interface...");

	ret = (*env)->NewStringUTF(env, req);

	free(req);

	return ret;
}

