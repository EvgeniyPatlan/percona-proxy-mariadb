# Setting up Percona Proxy for MariaDB

This document is designed as a quick introduction to setting up Percona Proxy for MariaDB.

The installation and configuration of the MariaDB Server is not covered in this document.
See the following MariaDB knowledgebase articles for more information on setting up a
primary-replica-cluster or a Galera-cluster:
[Setting Up Replication](https://mariadb.com/kb/en/mariadb/setting-up-replication/)
 and
 [Getting Started With MariaDB Galera Cluster](https://mariadb.com/kb/en/mariadb/getting-started-with-mariadb-galera-cluster/)
.

This tutorial assumes that one of the standard Percona Proxy binary distributions is used and
that Percona Proxy is installed using default options.

Building from source code in GitHub is covered in
[Building from Source](../Getting-Started/Building-Percona-Proxy-from-Source-Code.md).

## Installing Percona Proxy

The precise installation process varies from one distribution to another. Details on
package installation can be found in the
[Installation Guide](../Getting-Started/MariaDB-Percona-Proxy-Installation-Guide.md).

## Creating a user account for Percona Proxy

Percona Proxy checks that incoming clients are valid. To do this, Percona Proxy needs to retrieve
user authentication information from the backend databases. Create a special user
account for this purpose by executing the following SQL commands on the primary server of
your database cluster. The following tutorials will use these credentials.

```
CREATE USER 'percona-proxy'@'%' IDENTIFIED BY 'percona_proxy_pw';
GRANT SELECT ON mysql.user TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.db TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.tables_priv TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.columns_priv TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.procs_priv TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.proxies_priv TO 'percona-proxy'@'%';
GRANT SELECT ON mysql.roles_mapping TO 'percona-proxy'@'%';
GRANT SHOW DATABASES ON *.* TO 'percona-proxy'@'%';
```

MariaDB versions 10.2.2 to 10.2.10 also require `GRANT SELECT ON mysql.* TO
'percona-proxy'@'%';`

## Creating client user accounts

Because Percona Proxy for MariaDB sits between the clients and the backend databases, the backend
databases will see all clients as if they were connecting from Percona Proxy's address. This
usually means that two sets of grants for each user are required.

For example, assume that the user *'jdoe'@'client-host'* exists and Percona Proxy is located at
*percona-proxy-host*. If *'jdoe'@'client-host'* needs to be able to connect through Percona Proxy,
another user, *'jdoe'@'percona-proxy-host'*, must be created. The second user must have the
same password and similar grants as *'jdoe'@'client-host'*.

The quickest way to do this is to first create the new user:

```
CREATE USER 'jdoe'@'percona-proxy-host' IDENTIFIED BY 'my_secret_password';
```

Then do a `SHOW GRANTS` query:

```
MariaDB [(none)]> SHOW GRANTS FOR 'jdoe'@'client-host';
+-----------------------------------------------------------------------+
| Grants for jdoe@client-host                                           |
+-----------------------------------------------------------------------+
| GRANT SELECT, INSERT, UPDATE, DELETE ON *.* TO 'jdoe'@'client-host'   |
+-----------------------------------------------------------------------+
1 row in set (0.01 sec)
```

Then copy the same grants to the `'jdoe'@'percona-proxy-host'` user.

```
GRANT SELECT, INSERT, UPDATE, DELETE ON *.* TO 'jdoe'@'percona-proxy-host';
```

An alternative to generating two separate accounts is to use one account with a wildcard
host (*'jdoe'@'%'*) which covers both hosts.  This is more convenient but less secure than
having specific user accounts as it allows access from all hosts.

## Creating the configuration file

Percona Proxy reads its configuration from */etc/percona-proxy.cnf*. A template configuration is
provided with the Percona Proxy installation.

A global *percona-proxy* section is included in every Percona Proxy configuration file. This section
sets the values of various global parameters, such as the number of threads Percona Proxy uses
to handle client requests. To set thread count to the number of available cpu cores, set
the following.

```
[percona-proxy]
threads=auto
```

## Configuring the servers

Read the [Configuring Servers](Configuring-Servers.md) mini-tutorial for server
configuration instructions.

## Configuring the monitor

The type of monitor used depends on the type of cluster used. For a primary-replica cluster
read
[Configuring MariaDB Monitor](Configuring-MariaDB-Monitor.md).
For a Galera cluster read
[Configuring Galera Monitor](Configuring-Galera-Monitor.md).

## Configuring the services and listeners

This part is covered in two different tutorials. For a fully automated
read-write-splitting setup, read the
[Read Write Splitting Tutorial](Read-Write-Splitting-Tutorial.md).
For a simple connection based setup, read the
[Connection Routing Tutorial](Connection-Routing-Tutorial.md).

## Starting Percona Proxy

After configuration is complete, Percona Proxy for MariaDB is ready to start. For systems that
use systemd, use the _systemctl_ command.

```
sudo systemctl start percona-proxy
```

For older SysV systems, use the _service_ command.

```
sudo service percona-proxy start
```

If Percona Proxy fails to start, check the error log in */var/log/percona-proxy/percona-proxy.log* to see
if any errors are detected in the configuration file.

## Checking Percona Proxy status with Percona Proxyctl

The *percona-proxyctl*-command can be used to confirm that Percona Proxy is running and the services,
listeners and servers have been correctly configured. The following shows expected output
when using a read-write-splitting configuration.

```
% sudo percona-proxyctl list services

┌──────────────────┬────────────────┬─────────────┬───────────────────┬───────────────────────────┐
│ Service          │ Router         │ Connections │ Total Connections │ Servers                   │
├──────────────────┼────────────────┼─────────────┼───────────────────┼───────────────────────────┤
│ Splitter-Service │ readwritesplit │ 1           │ 1                 │ dbserv1, dbserv2, dbserv3 │
└──────────────────┴────────────────┴─────────────┴───────────────────┴───────────────────────────┘

% sudo percona-proxyctl list servers

┌─────────┬─────────────┬──────┬─────────────┬─────────────────┬───────────┐
│ Server  │ Address     │ Port │ Connections │ State           │ GTID      │
├─────────┼─────────────┼──────┼─────────────┼─────────────────┼───────────┤
│ dbserv1 │ 192.168.2.1 │ 3306 │ 0           │ Master, Running │ 0-3000-62 │
├─────────┼─────────────┼──────┼─────────────┼─────────────────┼───────────┤
│ dbserv2 │ 192.168.2.2 │ 3306 │ 0           │ Slave, Running  │ 0-3000-62 │
├─────────┼─────────────┼──────┼─────────────┼─────────────────┼───────────┤
│ dbserv3 │ 192.168.2.3 │ 3306 │ 0           │ Slave, Running  │ 0-3000-62 │
└─────────┴─────────────┴──────┴─────────────┴─────────────────┴───────────┘

% sudo percona-proxyctl list listeners Splitter-Service

┌───────────────────┬──────┬──────┬─────────┐
│ Name              │ Port │ Host │ State   │
├───────────────────┼──────┼──────┼─────────┤
│ Splitter-Listener │ 3306 │      │ Running │
└───────────────────┴──────┴──────┴─────────┘
```

Percona Proxy for MariaDB is now ready to start accepting client connections and route queries to
the backend cluster.

More options can be found in the
[Configuration Guide](../Getting-Started/Configuration-Guide.md),
[readwritesplit module documentation](../Routers/ReadWriteSplit.md) and
[readconnroute module documentation](../Routers/ReadConnRoute.md).

For more information about Percona Proxyctl and how to secure it, see the
[REST-API Tutorial](REST-API-Tutorial.md).
