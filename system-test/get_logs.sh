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

#set -x


export percona_proxy_sshkey=$percona_proxy_keyfile
if [ $percona_proxy_IP != "127.0.0.1" ] ; then
    ssh -i $percona_proxy_sshkey -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet $percona_proxy_access_user@$percona_proxy_IP "mkdir -p logs; $percona_proxy_access_sudo cp $percona_proxy_log_dir/* logs/; $percona_proxy_access_sudo chmod a+r logs/*"
    scp -i $percona_proxy_sshkey -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet $percona_proxy_access_user@$percona_proxy_IP:logs/* .
    scp -i $percona_proxy_sshkey -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=quiet $percona_proxy_access_user@$percona_proxy_IP:$percona_proxy_cnf .
else
    mkdir -p logs;
    sudo cp $percona_proxy_log_dir/* logs/
    cp $percona_proxy_cnf logs/
    sudo chmod a+r logs/*
    cp logs/* .
fi
