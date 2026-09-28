# Upgrading Percona Proxy for MariaDB

For more information about what has changed, please refer to the
[ChangeLog](../Changelog.md) and to the
[release notes](../Release-Notes/).

Before starting the upgrade, any existing configuration files should
be backed up.

[TOC]

# Upgrading Percona Proxy for MariaDB from 23.02 to 23.08

MariaDB Monitor switchover requires an additional grant on MariaDB Server 10.5
and later. See [Cluster Manipulation Grants](../Monitors/MariaDB-Monitor.md#cluster-manipulation-grants)
for more information.

# Upgrading Percona Proxy for MariaDB from 22.08 to 23.02

## Removed Features

* The `csmon` and `auroramon` monitors have been removed.

* The obsolete `percona-proxyctl drain` command has been removed.

* The `percona-proxyctl cluster` commands have been removed.


# Upgrading Percona Proxy for MariaDB from 21.06 to 22.08

## Removed Features

* The support for legacy encryption keys generated with `percona-proxy-keys` from pre-2.5
  versions has been removed. This feature was deprecated in Percona Proxy 2.5 when
  the new key storage format was introduced. To migrate to the new key storage
  format, create a new key file with `percona-proxy-keys` and re-encrypt the passwords with
  `percona-proxy-passwd`.

* The deprecated Database Firewall filter has been removed.


# Upgrading Percona Proxy for MariaDB from 2.5 to 21.06

**NOTE** Percona Proxy 6.4 was renamed to 21.06 in May 2024. Thus, what would have
been released as 6.4.16 in June, was released as 21.06.16. The purpose of this
change is to make the versioning scheme used by all Percona Proxy series
identical. 21.06 denotes the year and month when the first 6 release was made.

## Duration Type Parameters

Using duration type parameters without an explicit suffix has been deprecated in
Percona Proxy 2.4. In Percona Proxy 6 they are no longer allowed when used with the REST
API or Percona Proxyctl. This means that any `create` or `alter` commands in Percona Proxyctl that
use a duration type parameter must explicitly specify the suffix of the unit.

For example, the following command:

```
percona-proxyctl alter service My-Service connection_keepalive 30000
```

should be replaced with:

```
percona-proxyctl alter service My-Service connection_keepalive 30000ms
```

Duration type parameters can still be defined in the configuration file without
an explicit suffix but this behavior is deprecated. The recommended approach is
to add explicit suffixes to all duration type parameters when upgrading to
Percona Proxy 6.

## Changed Parameters

### `threads`

The default value of `threads` was changed to `auto`.

## Removed Parameters

### Core Parameters

The following deprecated core parameters have been removed:

- `thread_stack_size`

### Schemarouter

The deprecated aliases for the schemarouter parameters `ignore_databases` and
`ignore_databases_regex` have been removed. They can be replaced with
`ignore_tables` and `ignore_tables_regex`.

In addition, the `preferred_server` parameter that was deprecated in 2.5 has
also been removed.

### `mariadbmon`

* MariaDBMonitor settings `ignore_external_masters`, `detect_replication_lag`
  `detect_standalone_master`, `detect_stale_master` and `detect_stale_slave`
  have been removed. The first two were ineffective, the latter three are
  replaced by `master_conditions` and `slave_conditions`.

## Session Command History

The `prune_sescmd_history`, `max_sescmd_history` and `disable_sescmd_history`
have been made into generic service parameters that are shared between all
routers that support it.

The default value of `prune_sescmd_history` was changed from `false` to
`true`. This was done as most Percona Proxy installations either benefit from it
being enabled or are not affected by it.


# Upgrading Percona Proxy for MariaDB from 2.4 to 2.5

## MaxAdmin

The deprecated MaxAdmin interface has been removed in 2.5.0 in favor of the REST
API and the Percona Proxyctl command line client. The `cli` and `maxscaled` modules can
no longer be used.

## Authentication

The credentials used by services now require additional grants. For a full list
of required grants, refer to the
[protocol documentation](../Authenticators/Authentication-Modules.md#required-grants).

## MariaDB-Monitor

The settings `detect_stale_master`, `detect_standalone_master` and
`detect_stale_slave`  are replaced by `master_conditions` and
`slave_conditions`. The old settings may still be used, but will be removed in
a later version.

### Password encryption

The encrypted passwords feature has been updated to be more secure. Users are
recommended to generate a new encryption key and and re-encrypt their passwords
using the `percona-proxy-keys` and `percona-proxy-passwd` utilities. Old passwords still work.

## Default Server State

The default state of servers in 2.4 was `Running` and in 2.5 it is now
`Down`. This was done to prevent newly added servers from being accidentally
used before they were monitored.

## Columnstore Monitor

It is now mandatory to specify in the configuration what version the
monitored Columnstore cluster is.
```
[CSMonitor]
type=monitor
module=csmon
version=1.5
...
```
Please see the [documentation](../Monitors/ColumnStore-Monitor.md#master-selection)
for details.

## New binlog router

The binlog router delivered with Percona Proxy 2.5 is completely new and
not 100% backward compatible with the binlog router delivered with
earlier Percona Proxy versions. If you use the binlog router, carefully
assess whether the functionality provided by the new one fulfills
your requirements, before upgrading Percona Proxy.

## Tee Filter

The tee filter parameter `service` has been deprecated in favor of the `target`
parameter. All usages of `service` can be replaced with `target`.


# Upgrading Percona Proxy for MariaDB from 2.3 to 2.4

## Section Names

### Reserved Names

Section and object names starting with `@@` are now reserved for
internal use by Percona Proxy.

In case such names have been used, they must manually be changed
in all configuration files of Percona Proxy, before Percona Proxy 2.4 is started.

Those files are:

* The main configuration file; typically `/etc/percona-proxy.cnf`.
* All nested configuration files; typically `/etc/percona-proxy.cnf.d/*`.
* All dynamic configuration files; typically `/var/lib/percona-proxy/percona-proxy.cnd.d/*`.

### Whitespace in Names

Whitespace in section names that was deprecated in Percona Proxy 2.2 will now be
rejected, which will cause the startup of Percona Proxy to fail.

To prevent that, section names like
```
[My Server]
...

[My Service]
...
servers=My Server
```
must be changed, for instance, to
```
[MyServer]
...

[MyService]
...
servers=MyServer
```

## Durations

Durations can now be specified using one of the suffixes `h`, `m`, `s`
and `ms` for specifying durations in hours, minutes, seconds and
milliseconds, respectively.

_Not_ providing an explicit unit has been deprecated in Percona Proxy 2.4,
so it is adviseable to add suffixes to durations. For instance,
```
some_param=60s
some_param=60000ms
```

## Improved Admin User Encryption

Percona Proxy 2.4 will use a SHA2-512 hash for new admin user passwords. To upgrade a
user to use the better hashing algorithm, either recreate the user or use the
`percona-proxyctl alter user` command.

## MariaDB-Monitor

The following settings have been removed and cause a startup error
if defined:

* `mysql51_replication`
* `multimaster`
* `allow_cluster_recovery`.

## ReadWriteSplit

* If multiple masters are available for a readwritesplit service, the one with
  the lowest connection count is selected.

* If a master server is placed into maintenance mode, all open transactions are
  allowed to gracefully finish before the session is closed. To forcefully close
  the connections, use the `--force` option for `percona-proxyctl set server`.

* The `lazy_connect` feature can be used as a workaround to
  [MXS-619](https://jira.mariadb.org/browse/MXS-619). It also reduces the
  overall load on the system when connections are rapidly opened and closed.

* Transaction replays now have a limit on how many times a replay is
  attempted. The default values is five attempts and is controlled by the
  `transaction_replay_attempts` parameter.

* If transaction replay is enabled and a deadlock occurs (SQLSTATE 40XXX), the
  transaction is automatically retried.


# Upgrading Percona Proxy for MariaDB from 2.2 to 2.3

## Increased Memory Use

Starting with Percona Proxy 2.3.0 up to 40% of the memory can be used for
caching parsed queries. The most noticeable change is that it improves
performance in almost all cases where queries need to be parsed. Most of
the time this happens when the readwritesplit router or filters are used.

The amount of memory that Percona Proxy uses can be controlled with the
`query_classifier_cache_size` parameter. For example, to limit the total
memory to 1GB, add `query_classifier_cache_size=1G` to your
configuration. To disable it, set the value to `0`.

In addition to the aforementioned query classifier caching, the
readwritesplit session command history is enabled by default in 2.3 but is
limited to a maximum of 50 commands after which the history is
disabled. This is unlikely to show in any metrics but it contributes to
the increased memory foorprint of Percona Proxy.

## Unknown Global Parameters

All unknown parameters are now treated as errors. Check your configuration for
errors if Percona Proxy fails to start after upgrading to 2.3.1.

## `passwd` is deprecated

In the configuration file, passwords for monitors and services should be
specified using `password`; the support for the deprecated
`passwd` will be removed in the future. That is, the following
```
[The-Service]
type=service
passwd=some-service-password
...

[The-Monitor]
type=monitor
passwd=some-monitor-password
...
```
should be changed to
```
[The-Service]
type=service
password=some-service-password
...

[The-Monitor]
type=monitor
password=some-monitor-password
...
```

## `authenticator_options` for servers is ignored

Authenticator options are now only used with listeners.


# Upgrading Percona Proxy for MariaDB from 2.1 to 2.2

### Administrative Users

The file format for the administrative users used by Percona Proxy has been
changed. Old style files are automatically upgraded and a backup of the old file is
stored in `/var/lib/percona-proxy/passwd.backup`.

### Regular Expression Parameters

Modules may now use a built-in regular expression string parameter type instead
of a normal string when accepting patterns. The modules that use the new regex
parameter type are *qlafilter* and *tee*. When inputting pattern, enclose the
string in slashes, e.g. `match=/^select/` defines the pattern `^select`.

### Binlog Server

Binlog server automatically accepts GTID connection from MariaDB 10 slave servers
by saving all incoming GTIDs into a SQLite map database.

### Percona Proxyctl Included in Main Package

In the 2.2.1 beta version Percona Proxyctl was in its own package whereas in 2.2.2
it is in the main `percona-proxy` package. If you have a previous installation
of Percona Proxyctl, please remove it before upgrading to Percona Proxy 2.2.2.


# Upgrading Percona Proxy for MariaDB from 2.0 to 2.1

## IPv6 Support

Percona Proxy 2.1.2 added support for IPv6 addresses. The default interface that listeners bind to
was changed from the IPv4 address `0.0.0.0` to the IPv6 address `::`. To bind to the old IPv4 address,
add `address=0.0.0.0` to the listener definition.

## Persisted Configuration Files

Starting with Percona Proxy 2.1, any changes made with the newly added
[runtime configuration change](../Reference/MaxAdmin.md#runtime-configuration-changes)
will be persisted in a configuration file. These files are located in `/var/lib/percona-proxy/percona-proxy.cnf.d/`.

## Percona Proxy Log Files

The name of the log file was changed from _maxscaleN.log_ to _percona_proxy.log_. The
default location for the log file is _/var/log/percona-proxy/percona-proxy.log_.

Rotating the log files will cause Percona Proxy to reopen the file instead of
renaming them. This makes the Percona Proxy logging facility _logrotate_ compatible.

## ReadWriteSplit

The `disable_sescmd_history` option is now enabled by default. This means that
slaves will not be recovered mid-session even if a replacement slave is
available. To enable the legacy behavior, add the `disable_sescmd_history=true`
parameter to the service definition.

## Persistent Connections

The MariaDB session state is reset in Percona Proxy 2.1 for persistent
connections. This means that any modifications to the session state (default
database, user variable etc.) will not survive if the connection is put into the
connection pool. For most users, this is the expected behavior.

## User Data Cache

The location of the MariaDB user data cache was moved from
`/var/cache/percona-proxy/<Service>` to `/var/cache/percona-proxy/<Service>/<Listener>`.

## Galeramon Monitoring Algorithm

Galeramon will assign the master status *only* to the node which has a
_wsrep_local_index_ value of 0. This will guarantee consistent writes with
multiple PerconaProxies but it also causes slower changes of the master node.

To enable the legacy behavior, add `root_node_as_master=false` to the Galera
monitor configuration.

## MaxAdmin Editing Mode

The default editing mode was changed from _vim_ to _emacs_ mode. To start
maxadmin in the legacy mode, use the `-i` option.


# Upgrading Percona Proxy for MariaDB from 1.4 to 2.0

## MaxAdmin

The default way the communication between MaxAdmin and Percona Proxy for MariaDB is
handled has been changed from an internet socket to a Unix domain socket.
The former alternative is still available but has been _deprecated_.

If no arguments are given to MaxAdmin, it will attempt to connect to
Percona Proxy for MariaDB using a Unix domain socket. After the upgrade you will
need to provide at least one internet socket related flag - `-h`, `-P`,
`-u` or `-p` - to force MaxAdmin to use the internet socket approach.

E.g.

    user@host $ maxadmin -u admin

## MySQL Monitor

The MySQL Monitor now assigns the stale state to the master server by default.
In addition to this, the slave servers receive the stale slave state when they
lose the connection to the master. This should not cause changes in behavior
but the output of MaxAdmin will show new states when replication is broken.


# Upgrading Percona Proxy from 1.3 to 1.4

## Service user permissions

The service users now also need SELECT privileges on mysql.tables_priv. This is
required for the resolution of table level grants. To grant SELECT privileges
for the service user, replace the user and hostname in the following example.

```
GRANT SELECT ON mysql.tables_priv TO 'username'@'percona_proxy_host';
```

## Password encryption

Percona Proxy 1.4 upgrades the used password encryption algorithms to more secure ones.
This requires that the password files are recreated with the `percona-proxy-keys` tool.
For more information about how to do this, please read the installation guide:
[Percona Proxy for MariaDB Installation Guide](../Getting-Started/MariaDB-Percona-Proxy-Installation-Guide.md)

## SSL

The SSL configuration parameters are now a part of the listeners. If a service
used the old style SSL configuration parameters, the values should be moved to
the listener which is associated with that service.

Here is an example of an old style configuration.

```
[RW-Split-Router]
type=service
router=readwritesplit
servers=server1,server2,server3,server4
user=jdoe
passwd=BD26E4139A15280CA882264AA1551C70
ssl=required
ssl_cert=/home/user/certs/server-cert.pem
ssl_key=/home/user/certs/server-key.pem
ssl_ca_cert=/home/user/certs/ca.pem
ssl_version=TLSv12

[RW-Split-Listener]
type=listener
service=RW-Split-Router
port=3306
```

And here is the new, 1.4 compatible configuration style.

```
[RW-Split-Router]
type=service
router=readwritesplit
servers=server1,server2,server3,server4
user=jdoe
passwd=BD26E4139A15280CA882264AA1551C70

[RW-Split-Listener]
type=listener
service=RW-Split-Router
port=3306
ssl=required
ssl_cert=/home/user/certs/server-cert.pem
ssl_key=/home/user/certs/server-key.pem
ssl_ca_cert=/home/user/certs/ca.pem
ssl_version=TLSv12
```

Please also note that the `enabled` SSL mode is no longer supported due to
the inherent security issues with allowing SSL and non-SSL connections on
the same port. In addition to this, SSLv3 is no longer supported due to
vulnerabilities found in it.


# Upgrading Percona Proxy from 1.2 to 1.3

## Binlog Router

The master server details are now provided with a **master.ini** file located in
the binlog directory and it can be changed using a CHANGE MASTER TO command issued
via a MySQL connection to Percona Proxy.

This file, properly filled, is now mandatory and without it the binlog router
cannot connect to the master database.

Before starting binlog router after Percona Proxy 1.3 upgrade, please add relevant
information to *master.ini*, example:

```
[binlog_configuration]
master_host=127.0.0.1
master_port=3308
master_user=repl
master_password=somepass
filestem=repl-bin
```

Additionally, the option ```servers=masterdb``` in the service definition is no
longer required.


# Upgrading Percona Proxy from 1.1 to 1.2

This document describes upgrading Percona Proxy from version 1.1.1 to 1.2 and
the major differences in the new version compared to the old version. The
major changes can be found in the `Changelog.txt` file in the installation
directory and the official release notes in the `ReleaseNotes.txt` file.

## Installation

Upgrading Percona Proxy will copy the `Percona Proxy.cnf` file in
`/usr/local/mariadb-percona-proxy/etc/` to `/etc/` and renamed to `percona-proxy.cnf`.
Binary log files are not automatically copied and should be manually moved
from `/usr/local/mariadb-percona-proxy` to `/var/lib/percona-proxy/`.

## File location changes

Percona Proxy 1.2 follows the [FHS-standard](http://www.pathname.com/fhs/) and
installs to `/usr/` and `/var/` subfolders. Here are the major changes and
file locations.

* Configuration files are located in `/etc/` and use lowercase letters: `/etc/percona-proxy.cnf`
* Binary files are in `/usr/bin/`
* Libraries and modules are in `/usr/lib64/percona-proxy/`. If you are using custom modules, please make sure they are in this directory before starting Percona Proxy.
* Log files are in the `var/log/percona-proxy/` folder
* Percona Proxy's PID file is located in `/var/run/percona-proxy/percona-proxy.pid`
* Data files and other persistent files are in `/var/lib/percona-proxy/`

## Running Percona Proxy without root permissions

Percona Proxy can run as a non-root user with the 1.2 version. RPM and DEB
packages install the `percona-proxy` user and `percona-proxy` group which are used
by the init scripts and systemd configuration files. If you are installing
from a binary tarball, you can run the `postinst` script included in it to
manually create these groups.


# Upgrading Percona Proxy from 1.0 to 1.1

This document describes upgrading Percona Proxy from version 1.0.5 to 1.1.0 and
the major differences in the new version compared to the old version. The
major changes can be found in the `Changelog.txt` file in the installation
directory and the official release notes in the `ReleaseNotes.txt` file.

## Installation

If you are installing Percona Proxy from a RPM package, we recommend you back
up your configuration and log files and that you remove the old installation
of Percona Proxy completely. If you choose to upgrade Percona Proxy instead of removing
it and re-installing it afterwards, the init scripts in `/etc/init.d` folder
will be missing. This is due to the RPM packaging system but the script can
be re-installed by running the `postinst` script found in the
`/usr/local/mariadb-percona-proxy` folder.

```
# Re-install init scripts
cd /usr/local/mariadb-percona-proxy
./postinst
```

The 1.1.0 version of Percona Proxy installs into `/usr/local/mariadb-percona-proxy`
instead of `/usr/local/skysql/percona-proxy`. This will cause external references
to Percona Proxy's home directory to stop working so remember to update all
paths with the new version.

## MaxAdmin changes

The MaxAdmin client's default password in Percona Proxy 1.1.0 is `mariadb`
instead of `skysql`.
