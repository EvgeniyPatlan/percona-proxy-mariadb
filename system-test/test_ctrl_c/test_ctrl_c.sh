#!/bin/bash
#
# Copyright (c) 2016 MariaDB Corporation Ab
# Copyright (c) 2023 MariaDB plc, Finnish Branch
#
# Use of this software is governed by the Business Source License included
# in the LICENSE.TXT file and at www.mariadb.com/bsl11.
#
# Change Date: 2026-09-21
#
# On the date above, in accordance with the Business Source License, use
# of this software will be governed by version 2 or later of the General
# Public License.
#

sudo systemctl stop percona-proxy || sudo service percona-proxy stop

hm=`pwd`
$hm/start_killer.sh &
if [ $? -ne 0 ] ; then
        exit 1
fi

T="$(date +%s)"

# Normally SystemD would create the PID directory /var/run/percona-proxy/ for us but
# since we're starting Percona Proxy manually, we need to make sure it points to a
# location that the percona-proxy user can write into.
/usr/bin/sudo ASAN_OPTIONS=detect_leaks=0 percona-proxy -d -U percona-proxy --piddir=/tmp/
if [ $? -ne 0 ] ; then
	exit 1
fi

T="$(($(date +%s)-T))"
echo "Time in seconds: ${T} (including 5 seconds before kill)"

if [ "$T" -lt 10 ] ; then
        echo "PASSED"
        exit 0
else
        echo "FAILED"
        exit 1
fi

