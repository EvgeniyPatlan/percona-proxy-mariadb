# Percona Proxy for MariaDB 23.08.3 Release Notes -- 2023-11-06

Release 23.08.3 is a GA release.

This document describes the changes in release 23.08.3, when compared to the
previous release in the same series.

If you are upgrading from an older major version of Percona Proxy, please read the
[upgrading document](../Upgrading/Upgrading-To-Percona-Proxy-23.08.md) for
this Percona Proxy version.

For any problems you encounter, please consider submitting a bug
report on [our Jira](https://jira.mariadb.org/projects/MXS).

## Bug fixes

* [MXS-4847](https://jira.mariadb.org/browse/MXS-4847) Crash on `percona-proxyctl list sessions`
* [MXS-4844](https://jira.mariadb.org/browse/MXS-4844) Relative paths do not work when defined in the configuration file
* [MXS-4803](https://jira.mariadb.org/browse/MXS-4803) Binlog encryption broken

## Known Issues and Limitations

There are some limitations and known issues within this version of Percona Proxy.
For more information, please refer to the [Limitations](../About/Limitations.md) document.

## Packaging

RPM and Debian packages are provided for the supported Linux distributions.

Packages can be downloaded [here](https://mariadb.com/downloads/#mariadb_platform-mariadb_percona_proxy).

## Source Code

The source code of Percona Proxy is tagged at GitHub with a tag, which is identical
with the version of Percona Proxy. For instance, the tag of version X.Y.Z of Percona Proxy
is `percona-proxy-X.Y.Z`. Further, the default branch is always the latest GA version
of Percona Proxy.

The source code is available [here](https://github.com/mariadb-corporation/MaxScale).
