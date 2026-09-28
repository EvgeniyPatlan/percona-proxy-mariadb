#!/bin/bash

#
# This script is run after each test block. It kills the Percona Proxy process.
#

percona_proxy_dir=$PERCONA_PROXY_DIR

test -z "$PERCONA_PROXY_DIR" && exit 1

pkill '^percona-proxy$'

for ((i=0;i<100;i++))
do
    pgrep '^percona-proxy$' &> /dev/null || exit 0
    sleep 0.1
done

# If it wasn't dead before, now it is
pgrep '^percona-proxy$' &> /dev/null && pkill -6 '^percona-proxy$'

exit 0
