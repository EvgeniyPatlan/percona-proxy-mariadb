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

script=`basename "$0"`

source=$src_dir/masking/$1/masking_rules.json
target=${percona_proxy_000_whoami}@${percona_proxy_000_network}:/home/${percona_proxy_000_whoami}/masking_rules.json

if [ ${percona_proxy_000_network} != "127.0.0.1" ] ; then
	scp -i $percona_proxy_000_keyfile -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $source $target
        ssh -i $percona_proxy_000_keyfile -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ${percona_proxy_000_whoami}@${percona_proxy_000_network} chmod a+r /home/${percona_proxy_000_whoami}/masking_rules.json
else
	cp $source /home/${percona_proxy_000_whoami}/masking_rules.json
fi

if [ $? -ne 0 ]
then
    echo "error: Could not copy rules file to percona-proxy host."
    exit 1
fi

echo $source copied to $target, restarting Percona Proxy

ssh  -i $percona_proxy_000_keyfile -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ${percona_proxy_000_whoami}@${percona_proxy_000_network} 'sudo systemctl restart percona-proxy'

# [Read Connection Listener Master] in cnf/percona-proxy.percona-proxy.cnf.template.$1
port=4008

$src_dir/mysqltest_driver.sh $1 $src_dir/masking/$1 $port $percona_proxy_user $percona_proxy_password
