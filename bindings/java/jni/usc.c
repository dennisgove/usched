/**
 * @file usc.c
 * @brief uSched JNI Interface
 *        uSched JNI interface - Client
 *
 * Date: 12-01-2015
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
#include <jni.h>

#include "Usched.h"

#include <usched/lib.h>

/* Static globals */
static struct usched_entry *_usc_entry_list = NULL;
static size_t _usc_nmemb = 0, _usc_cur = 0;

JNIEXPORT void JNICALL Java_Usched_nativeInit(JNIEnv *env, jobject obj) {
	usched_init();
}

JNIEXPORT void JNICALL Java_Usched_nativeDestroy(JNIEnv *env, jobject obj) {
	usched_destroy();
}

JNIEXPORT jstring JNICALL Java_Usched_nativeTest(JNIEnv *env, jobject obj) {
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, "Testing uSched interface...");

	return result;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeOptSetRemoteHostname(
		JNIEnv *env,
		jobject obj,
		jstring hostname)
{
	const char *n_hostname = (*env)->GetStringUTFChars(env, hostname, 0);

	if (usched_opt_set_remote_hostname((char *) n_hostname) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, hostname, n_hostname);

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeOptSetRemotePort(
		JNIEnv *env,
		jobject obj,
		jstring port)
{
	const char *n_port = (*env)->GetStringUTFChars(env, port, 0);

	if (usched_opt_set_remote_port((char *) n_port) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, port, n_port);

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeOptSetRemoteUsername(
		JNIEnv *env,
		jobject obj,
		jstring username)
{
	const char *n_username = (*env)->GetStringUTFChars(env, username, 0);

	if (usched_opt_set_remote_username((char *) n_username) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, username, n_username);

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeOptSetRemotePassword(
		JNIEnv *env,
		jobject obj,
		jstring password)
{
	const char *n_password = (*env)->GetStringUTFChars(env, password, 0);

	if (usched_opt_set_remote_password((char *) n_password) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, password, n_password);

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeRequest(
		JNIEnv *env,
		jobject obj,
		jstring request)
{
	const char *n_request = (*env)->GetStringUTFChars(env, request, 0);

	if (usched_request((char *) n_request) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, request, n_request);

	return JNI_TRUE;
}

JNIEXPORT jlongArray JNICALL Java_Usched_nativeResultGetRun(
		JNIEnv *env,
		jobject obj)
{
	uint64_t *entry_list;
	size_t nmemb;
	jlongArray result;

	usched_result_get_run(&entry_list, &nmemb);

	result = (*env)->NewLongArray(env, nmemb);

	(*env)->SetLongArrayRegion(env, result, 0, nmemb, (jlong *) entry_list);

	return result;
}

JNIEXPORT jlongArray JNICALL Java_Usched_nativeResultGetStop(
		JNIEnv *env,
		jobject obj)
{
	uint64_t *entry_list;
	size_t nmemb;
	jlongArray result;

	usched_result_get_stop(&entry_list, &nmemb);

	result = (*env)->NewLongArray(env, nmemb);

	(*env)->SetLongArrayRegion(env, result, 0, nmemb, (jlong *) entry_list);

	return result;
}

JNIEXPORT void JNICALL Java_Usched_nativeResultGetShowInit(
		JNIEnv *env,
		jobject obj)
{
	usched_result_get_show(&_usc_entry_list, &_usc_nmemb);

	_usc_cur = 0;
}

JNIEXPORT void JNICALL Java_Usched_nativeResultGetShowDestroy(
		JNIEnv *env,
		jobject obj)
{
	_usc_entry_list = NULL;
	_usc_nmemb = 0;
	_usc_cur = 0;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowNmemb(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_nmemb;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowCur(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_cur;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeResultGetShowNext(
		JNIEnv *env,
		jobject obj)
{
	if ((_usc_cur + 1) >= _usc_nmemb)
		return JNI_FALSE;

	_usc_cur ++;

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Usched_nativeResultGetShowPrev(
		JNIEnv *env,
		jobject obj)
{
	if (_usc_cur <= 0)
		return JNI_FALSE;

	_usc_cur --;

	return JNI_TRUE;
}

JNIEXPORT jlong JNICALL Java_Usched_nativeResultGetShowId(
		JNIEnv *env,
		jobject obj)
{
	return (jlong) _usc_entry_list[_usc_cur].id;
}

JNIEXPORT jstring JNICALL Java_Usched_nativeResultGetShowUsername(
		JNIEnv *env,
		jobject obj)
{
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, _usc_entry_list[_usc_cur].username);

	return result;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowUID(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_entry_list[_usc_cur].uid;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowGID(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_entry_list[_usc_cur].gid;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowTrigger(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_entry_list[_usc_cur].trigger;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowStep(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_entry_list[_usc_cur].step;
}

JNIEXPORT jint JNICALL Java_Usched_nativeResultGetShowExpire(
		JNIEnv *env,
		jobject obj)
{
	return (jint) _usc_entry_list[_usc_cur].expire;
}

JNIEXPORT jstring JNICALL Java_Usched_nativeResultGetShowCmd(
		JNIEnv *env,
		jobject obj)
{
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, _usc_entry_list[_usc_cur].subj);

	return result;
}

JNIEXPORT void JNICALL Java_Usched_nativeResultFreeRun(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_run();
}

JNIEXPORT void JNICALL Java_Usched_nativeResultFreeStop(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_stop();
}

JNIEXPORT void JNICALL Java_Usched_nativeResultFreeShow(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_show();
}

JNIEXPORT jstring JNICALL Java_Usched_nativeUsageErrorStr(
		JNIEnv *env,
		jobject obj,
		jint error)
{
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, usched_usage_error_str((int) error));

	return result;
}

