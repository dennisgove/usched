#!/bin/sh
#
# @file usched.sh
# @brief uSched
#        uSched flush/start/stop script - Shell implementation
#
# Date: 15-02-2015
# 
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# This file is part of usched.
#
# usched is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# usched is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with usched.  If not, see <http://www.gnu.org/licenses/>.
#

## Arguments data ##
ARGC=${#}
ARG0=${0}; ARG1=${1}; ARG2=${2};

## Config data ##
CONFIG_USCHED_QUIET=0
CONFIG_USCHED_DAEMON_BIN="/usr/sbin/usd"
CONFIG_USCHED_EXEC_BIN="/usr/sbin/use"
CONFIG_USCHED_MONITOR_BIN="/usr/sbin/usm"
CONFIG_USCHED_DAEMON_PID_FILE="/var/run/usched_usd.pid"
CONFIG_USCHED_EXEC_PID_FILE="/var/run/usched_use.pid"

## Status ##
EXIT_SUCCESS=0
EXIT_FAILURE=1

## Implementation ##
print_info() {
	if [ ${CONFIG_USCHED_QUIET} -ne 1 ]; then
		printf "${1}"
	fi
}

usage() {
	print_info "Usage: ${ARG0} flush|reload|start|stop [quiet]\n"
	exit ${EXIT_FAILURE}
}

args_check() {
	if [ ${ARGC} -lt 1 ]; then 
		usage
	elif [ ${ARGC} -eq 2 ]; then
		if [ ${ARG2} != "quiet" ]; then
			usage
		fi

		CONFIG_USCHED_QUIET=1
	elif [ ${ARGC} -gt 2 ]; then
		usage
	fi
}

perms_check() {
	if [ `id -u` -ne 0 ]; then
		print_info "You need to be root to perform that operation.\n"
		exit ${EXIT_FAILURE}
	fi
}

op_flush() {
	print_info "Flush operation status: "

	kill -USR1 `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`

	return ${?}
}

op_reload() {
	print_info "Reload operation status: "

	kill -HUP `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`

	return ${?}
}

op_start() {
	print_info "Start operation status: "

	${CONFIG_USCHED_MONITOR_BIN} -p ${CONFIG_USCHED_DAEMON_PID_FILE} -r -S ${CONFIG_USCHED_DAEMON_BIN}

	if [ ${?} -ne 0 ]; then
		return ${?}
	fi

	${CONFIG_USCHED_MONITOR_BIN} -p ${CONFIG_USCHED_EXEC_PID_FILE} -r -S ${CONFIG_USCHED_EXEC_BIN}

	return ${?}
}

op_stop() {
	print_info "Stop operation status: "

	kill -TERM `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`
	kill -TERM `cat ${CONFIG_USCHED_EXEC_PID_FILE}`

	while [ -f ${CONFIG_USCHED_DAEMON_PID_FILE} ]; do sleep 1; done
	while [ -f ${CONFIG_USCHED_EXEC_PID_FILE} ]; do sleep 1; done

	return 0
}

process_op() {
	case "${ARG1}" in
		flush)
			op_flush
			return ${?}
			;;
		reload)
			op_reload
			return ${?}
			;;
		start)
			op_start
			return ${?}
			;;
		stop)
			op_stop
			return ${?}
			;;
		*)
			echo "Invalid operation: ${ARG1}"
			usage
	esac
}

main() {
	args_check

	perms_check

	process_op

	status=${?}

	if [ ${status} -ne ${EXIT_SUCCESS} ]; then
		print_info "Failed.\n"
	else
		print_info "Success.\n"
	fi

	exit ${status}
}

## Entry point ##
main
