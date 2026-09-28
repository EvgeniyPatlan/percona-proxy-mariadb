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

###
## @file bug567.sh Regression case for the bug "Crash if files from /dev/shm/ removed"
## - try to remove everythign from /dev/shm/$percona_proxy_pid
## check if Maxscale is alive

export ssl_options="--ssl-cert=$src_dir/ssl-cert/client.crt --ssl-key=$src_dir/ssl-cert/client.key --ssl-verify-server-cert=0"

#pid=`ssh -i $percona_proxy_sshkey -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ${percona_proxy_000_whoami}@${percona_proxy_000_network} "pgrep percona-proxy"`
#echo "Maxscale pid is $pid"
echo "removing log directory from /dev/shm/"
if [ ${percona_proxy_000_network} != "127.0.0.1" ] ; then
	ssh -i ${percona_proxy_000_keyfile} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ${percona_proxy_000_whoami}@${percona_proxy_000_network} "sudo rm -rf /dev/shm/percona-proxy/*"
else
	sudo rm -rf /dev/shm/percona-proxy/*
fi
sleep 1
echo "checking if Maxscale is alive"
echo "show databases;" | mariadb -u$node_user -p$node_password -h ${percona_proxy_000_network} -P 4006 $ssl_options
res=$?

exit $res

