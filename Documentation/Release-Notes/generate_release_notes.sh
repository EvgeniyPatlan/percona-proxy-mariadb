#!/bin/bash

cd $(dirname $(realpath $0))

major="`cd ../../ && cmake -P ./VERSION.cmake -L|grep 'PERCONA_PROXY_VERSION_MAJOR'|sed 's/.*=//'`"
minor="`cd ../../ && cmake -P ./VERSION.cmake -L|grep -v 'PERCONA_PROXY_VERSION_MINOR_NUM' | grep 'PERCONA_PROXY_VERSION_MINOR'|sed 's/.*=//'`"
patch="`cd ../../ && cmake -P ./VERSION.cmake -L|grep 'PERCONA_PROXY_VERSION_PATCH'|sed 's/.*=//'`"
maturity="`cd ../../ && cmake -P ./VERSION.cmake -L|grep 'PERCONA_PROXY_MATURITY'|sed 's/.*=//'`"

VERSION="${major}.${minor}.${patch}"

# For version 6, this is just the major version. For other versions, it
# is $major.$minor. Needs to be updated whenever a new major release is
# out or if the versioning scheme for Percona Proxy changes.
upgrade_version="$major.$minor"

cat <<EOF > Percona Proxy-$VERSION-Release-Notes.md
# Percona Proxy for MariaDB ${VERSION} Release Notes

Release ${VERSION} is a ${maturity} release.

This document describes the changes in release ${VERSION}, when compared to the
previous release in the same series.

If you are upgrading from an older major version of Percona Proxy, please read the
[upgrading document](../Upgrading/Upgrading-To-Percona-Proxy-${upgrade_version}.md) for
this Percona Proxy version.

For any problems you encounter, please consider submitting a bug
report on [our Jira](https://jira.mariadb.org/projects/MXS).

`../list_fixed.sh ${VERSION}`

## Known Issues and Limitations

There are some limitations and known issues within this version of Percona Proxy.
For more information, please refer to the [Limitations](../About/Limitations.md) document.

## Packaging

RPM and Debian packages are provided for the supported Linux distributions.

Packages can be downloaded [here](https://mariadb.com/downloads/#mariadb_platform-mariadb_percona_proxy).

## Source Code

The source code of Percona Proxy is tagged at GitHub with a tag, which is identical
with the version of Percona Proxy. For instance, the tag of version X.Y.Z of Percona Proxy
is \`percona-proxy-X.Y.Z\`. Further, the default branch is always the latest GA version
of Percona Proxy.

The source code is available [here](https://github.com/EvgeniyPatlan/percona-proxy-mariadb).
EOF
