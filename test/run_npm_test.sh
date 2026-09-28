#!/bin/bash

# This script builds and installs Percona Proxy, starts a MariaDB cluster and runs any
# tests that define a `npm test` target
#
# This is definitely not the most efficient way to test the binaries but it's a
# guaranteed method of creating a consistent and "safe" testing environment.
#
# Test run customization:
#
# NUMCPU:         The number of parallel build jobs to use.
# SKIP_SHUTDOWN:  If set, leaves the docker-compose setup up.
# DOCKER:         The "docker" command.
# DOCKER_COMPOSE: The "docker-compose" command.
#
if [ $# -lt 3 ]
then
    echo "USAGE: $0 <Percona Proxy sources> <test sources> <test directory>"
    exit 1
fi

if [ -z "$NUMCPU" ]
then
    export NUMCPU=$(grep -c processor /proc/cpuinfo)
fi

if [ -z "$DOCKER" ]
then
    DOCKER=docker
fi

if [ -z "$DOCKER_COMPOSE" ]
then
    DOCKER_COMPOSE=docker-compose
fi

srcdir=$1
testsrc=$2
testdir=$3

percona_proxy_dir=$PWD/

rm -f $percona_proxy_dir/percona_proxy1.output $percona_proxy_dir/log/percona-proxy/percona-proxy.log

# Create the test directories
mkdir -p $percona_proxy_dir $testdir

# Copy the common test files (start/stop scripts etc.)
cp -p -t $testdir -r $srcdir/test/*

# Copy test sources to test workspace
cp -p -t $testdir -r $testsrc/*

# Required by Percona Proxyctl (not super pretty)
cp -p -t $testdir/.. $srcdir/VERSION*.cmake

# This avoids running npm as root if we're executing the tests as root (Percona Proxyctl specific)
(cd $testdir && test -f configure_version.cmake && cmake -P configure_version.cmake)

# Copy required docker-compose files to the Percona Proxy directory and bring MariaDB
# servers up. This is an asynchronous process.
cd $percona_proxy_dir
cp -p -t $percona_proxy_dir -r $srcdir/test/*
$DOCKER_COMPOSE up -d || exit 1

# Install dependencies
cd $testdir
npm install || exit 1

# UBSAN won't abort the process without these options
export UBSAN_OPTIONS=abort_on_error=1:print_stacktrace=1

# Configure and install Percona Proxy
cd $percona_proxy_dir
cmake $srcdir -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$percona_proxy_dir \
      -DMAXSCALE_VARDIR=$percona_proxy_dir || exit 1

make -j $NUMCPU install || exit 1

# Create required directories (we could run the postinst script but it's a bit too invasive)
mkdir -p -m 0755 $percona_proxy_dir/{lib,lib64,share,log,cache,run}/percona-proxy
mkdir -p -m 0755 $percona_proxy_dir/bin
mkdir -p -m 0755 $percona_proxy_dir/share/doc/Percona Proxy/percona-proxy

# This variable is used to start and stop Percona Proxy before each test
export PERCONA_PROXY_DIR=$percona_proxy_dir

# Wait until the servers are up
cd $percona_proxy_dir
for node in server1 server2 server3 server4
do
    printf "Waiting for $node to start... "
    for ((i=0; i<60; i++))
    do
        $DOCKER exec -i $node mysql -umaxuser -pmaxpwd -e "select 1" >& /dev/null && break
        sleep 1
    done

    $DOCKER exec -i $node mysql -umaxuser -pmaxpwd -e "select 1" >& /dev/null

    if [ $? -ne 0 ]
    then
        echo "failed to start $node, error is:"
        $DOCKER exec -i $node mysql -umaxuser -pmaxpwd -e "select 1"
        exit 1
    else
        echo "Done!"
    fi
done

# Go to the test directory
cd $testdir

# Make sure no stale processes of files are left from an earlier run
./stop_percona_proxy.sh
./start_percona_proxy.sh

# Run tests
npm test
rval=$?

./stop_percona_proxy.sh

# Stop MariaDB servers
if [ -z  "$SKIP_SHUTDOWN" ]
then
    cd $percona_proxy_dir
    $DOCKER_COMPOSE down -v
fi

exit $rval
