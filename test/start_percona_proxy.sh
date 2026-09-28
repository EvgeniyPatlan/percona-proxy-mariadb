#!/bin/bash

#
# This script is run before each test block. It starts Percona Proxy and waits for it
# to become responsive.
#

percona_proxy_dir=$PERCONA_PROXY_DIR

test -z "$PERCONA_PROXY_DIR" && exit 1

test -f $percona_proxy_dir/data/run/percona-proxy.pid && kill -9 $(cat $percona_proxy_dir/data/run/percona-proxy.pid)

rm -r $percona_proxy_dir/data/

mkdir -m 0755 -p $percona_proxy_dir/data/{lib,cache,language,run,percona-proxy.cnf.d}/

if [ "`whoami`" == "root" ]
then
    user_opt="-U root"
fi

# Start Percona Proxy
$percona_proxy_dir/bin/percona-proxy $user_opt -f $percona_proxy_dir/percona-proxy.cnf &>> $percona_proxy_dir/percona-proxy.output || exit 1

# Wait for Percona Proxy to start
for ((i=0;i<150;i++))
do
    curl -s -f -u admin:mariadb 127.0.0.1:8989/v1/servers >& /dev/null && exit 0
    sleep 0.1
done

# Percona Proxy failed to start, exit with an error
exit 1
