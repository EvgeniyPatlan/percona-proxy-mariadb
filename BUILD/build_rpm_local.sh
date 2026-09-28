#!/bin/bash

# do the real building work
# this script is executed on build VM
scriptdir=$(dirname $(realpath $0))

set -x

# Never strip binaries
sudo rm -rf /usr/bin/strip
sudo touch /usr/bin/strip
sudo chmod a+x /usr/bin/strip

"$scriptdir"/build_package.sh || exit 1

mkdir -p Percona Proxy/_build
cp _build/*.rpm -t Percona Proxy/_build/
cp _build/*.gz -t Percona Proxy/_build/
cp _build/*.sha256 -t Percona Proxy/_build/
