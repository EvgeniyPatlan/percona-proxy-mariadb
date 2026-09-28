# Percona Proxy for MariaDB

Percona Proxy for MariaDB is a database proxy that forwards database statements to
one or more servers, using configurable rules, a semantic understanding of the
statements and the roles of the servers in the backend cluster. It provides load
balancing and high availability transparently to the applications, and a plugin
architecture for protocols, routers, filters, monitors and authenticators.

## Status

Early development, version 1.0.0. The product is derived from MariaDB MaxScale
23.08.12. The source tree, the binaries and the packages carry the Percona Proxy
names; `tools/rename.sh` documents how that rename was produced from the upstream
import. See `NOTICE` for the provenance and the licensing.

The bundled SQLite parser keeps the upstream names, on both sides of its interface,
because it is third-party code that is not renamed.

The GUI icons and the wordmark in `gui/public` and `gui/share/assets` are plain placeholders,
not the final artwork.

An installation that has not been migrated is still read: when `/etc/percona-proxy.cnf`
does not exist, `/etc/maxscale.cnf` is used and a warning is logged.
`percona-proxy-migrate` converts a MaxScale configuration into a Percona Proxy one.

## License

GNU General Public License, version 2 or later. See `COPYING`.

MariaDB MaxScale 23.08, from which this product is derived, was published under the
Business Source License 1.1 with the Change Date 2026-09-21 and the GNU General Public
License version 2 or later as the Change License. That date has passed, so this code is
available under the GPL.

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

The documentation in `Documentation/` is inherited from upstream. The product names in
it were renamed with the rest of the tree, but it has not yet been reviewed for
statements that only hold for MariaDB MaxScale, such as links to the MariaDB
knowledge base.
