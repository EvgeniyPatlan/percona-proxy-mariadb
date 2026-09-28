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

# $1 - test name
# $2 - time mark (in case of periodic logs copying)

if [ -z $1 ]; then
	echo "Test name missing"
	logs_dir="LOGS/nomane"
else
	if [ -z $2 ]; then
                logs_dir="LOGS/$1"
	else
		logs_dir="LOGS/$1/$2"
	fi
#	rm -rf $logs_dir
fi


echo "Creating log dir in workspace $logs_dir"
mkdir -p $logs_dir
if [ $? -ne 0 ]; then
        echo "Error creating log dir"
fi

echo "log_dir:         $logs_dir"
echo "percona_proxy_sshkey: $percona_proxy_000_keyfile"
echo "percona_proxy_IP:     $percona_proxy_000_network"

if [ $percona_proxy_IP != "127.0.0.1" ] ; then
    ssh -i ${percona_proxy_000_keyfile} -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet ${percona_proxy_000_whoami}@${percona_proxy_000_network} "rm -rf logs; mkdir logs; ${percona_proxy_000_access_sudo} cp ${percona_proxy_log_dir}/*.log logs/; ${percona_proxy_000_access_sudo} cp /tmp/core* logs; ${percona_proxy_000_access_sudo} chmod 777 -R logs"
    scp -i ${percona_proxy_000_keyfile} -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet ${percona_proxy_000_whoami}@${percona_proxy_000_network}:logs/* $logs_dir
    if [ $? -ne 0 ]; then
	echo "Error copying Maxscale logs"
    fi
    scp -i ${percona_proxy_000_keyfile} -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet ${percona_proxy_000_whoami}@${percona_proxy_000_network}:$percona_proxy_cnf $logs_dir
    chmod a+r $logs_dir/*
else
    sudo cp $percona_proxy_log_dir/*.log $logs_dir
    sudo cp /tmp/core* $logs_dir
    sudo cp $percona_proxy_cnf $logs_dir
    sudo chmod a+r $logs_dir/*
fi

if [ -z $logs_publish_dir ] ; then
	echo "logs are in workspace only"
else
	echo "Logs publish dir is $logs_publish_dir"
	rsync -a --no-o --no-g LOGS $logs_publish_dir
fi

for i in `find $logs_dir -name 'core*'`
do
    test -e $i && echo "Test failed: core files generated" && exit 1
done
