# Percona Proxy for MariaDB

Percona Proxy for MariaDB is a database proxy that forwards database statements to
one or more servers, using configurable rules, a semantic understanding of the
statements and the roles of the servers in the backend cluster. It provides load
balancing and high availability transparently to the applications, and a plugin
architecture for protocols, routers, filters, monitors and authenticators.

## Status

Early development. The product is derived from Percona Proxy for MariaDB 23.08.12 and is
being renamed; until that work lands, parts of the tree, the binaries and the
packages still carry the upstream names. See `NOTICE` for the provenance and the
licensing.

## License

GNU General Public License, version 2 or later. See `COPYING`.

Percona Proxy 23.08 was published under the Business Source License 1.1 with the Change
Date 2026-09-21 and the GNU General Public License version 2 or later as the Change
License. That date has passed, so this code is available under the GPL.

## Building

The build is CMake based and needs the `mariadb-connector-c` submodule:

```bash
git submodule update --init
mkdir ../build && cd ../build
../percona-proxy-mariadb/BUILD/install_build_deps.sh
cmake ../percona-proxy-mariadb -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

Packages are built from the packaging in `BUILD/percona`:

```bash
BUILD/percona/percona_proxy_builder.sh --builddir=<dir> --install_deps=1 --get_sources=1
BUILD/percona/percona_proxy_builder.sh --builddir=<dir> --build_src_rpm=1
BUILD/percona/percona_proxy_builder.sh --builddir=<dir> --build_rpm=1
```

`BUILD/percona/verify_packages.sh` installs the resulting packages on every
supported platform and checks that queries are routed through two real MariaDB
servers. `BUILD/percona/docker` holds the container images.

## Documentation

The documentation in `Documentation/` is inherited from upstream and still uses the
upstream names.
