# Percona Proxy for MariaDB 23.08.10 Release Notes

Release 23.08.10 is a GA release.

This document describes the changes in release 23.08.10, when compared to the
previous release in the same series.

If you are upgrading from an older major version of Percona Proxy, please read the
[upgrading document](../Upgrading/Upgrading-To-Percona-Proxy-23.08.md) for
this Percona Proxy version.

For any problems you encounter, please consider submitting a bug
report on [our Jira](https://jira.mariadb.org/projects/MXS).

## Bug fixes

* [MXS-5618](https://jira.mariadb.org/browse/MXS-5618) Maxctrl interactive mode doesn't use --tls-verify-server-cert=false
* [MXS-5613](https://jira.mariadb.org/browse/MXS-5613) The logout screen is shown when accessing the Percona Proxy GUI login view.
* [MXS-5608](https://jira.mariadb.org/browse/MXS-5608) optimistic_trx causes a query to hang
* [MXS-5599](https://jira.mariadb.org/browse/MXS-5599) Processing of conditional headers is incorrect
* [MXS-5598](https://jira.mariadb.org/browse/MXS-5598) Percona Proxyctl fails to read large inputs from stdin
* [MXS-5597](https://jira.mariadb.org/browse/MXS-5597) admin_oidc_url is documented to not be dynamic when in fact it is
* [MXS-5590](https://jira.mariadb.org/browse/MXS-5590) REST-API always sends a Connection: close header
* [MXS-5588](https://jira.mariadb.org/browse/MXS-5588) Signal 11 crash when enabling causal reads with Galera
* [MXS-5582](https://jira.mariadb.org/browse/MXS-5582) Add a Service with a CLUSTER as its target breaks CONFIG SYNC
* [MXS-5577](https://jira.mariadb.org/browse/MXS-5577) Aborted connection on backend mariadb with persistpool percona-proxy
* [MXS-5576](https://jira.mariadb.org/browse/MXS-5576) Maxctrl config permission check error message is misleading
* [MXS-5567](https://jira.mariadb.org/browse/MXS-5567) Wrong password in interactive mode is only seen after the first command
* [MXS-5566](https://jira.mariadb.org/browse/MXS-5566) --secretsdir has no default value
* [MXS-5563](https://jira.mariadb.org/browse/MXS-5563) Using PKCS#1 private key in the REST-API results in cryptic errors
* [MXS-5556](https://jira.mariadb.org/browse/MXS-5556) Trailing parts of large session command are not routed correctly
* [MXS-5542](https://jira.mariadb.org/browse/MXS-5542) kafkacdc commits offsets when it probes GTIDs from Kafka
* [MXS-5541](https://jira.mariadb.org/browse/MXS-5541) Logs Archive page doesn't show useful API error
* [MXS-5525](https://jira.mariadb.org/browse/MXS-5525) Masking with functions uses wrong rule settings

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
