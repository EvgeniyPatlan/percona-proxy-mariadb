# Percona Proxy for MariaDB Installation Guide

We recommend to install Percona Proxy on a separate server, to ensure that there
can be no competition of resources between Percona Proxy and a MariaDB Server that
it manages.

## Install Percona Proxy for MariaDB From MariaDB Repositories

The recommended approach is to use [the MariaDB package
repository](https://mariadb.com/kb/en/library/mariadb-package-repository-setup-and-usage/)
to install Percona Proxy. After enabling the repository by following the
instructions, Percona Proxy can be installed with the following commands.

* For RHEL/Rocky Linux/Alma Linux, use `dnf install percona-proxy`.

* For Debian and Ubuntu, run `apt update` followed by `apt install percona-proxy`.

* For SLES, use `zypper install percona-proxy`.

## Install Percona Proxy for MariaDB From a RPM/DEB Package

Download the correct Percona Proxy package for your CPU architecture and operating
system from [the MariaDB Downloads
page](https://mariadb.com/downloads/community/percona-proxy/). Percona Proxy can be
installed with the following commands.

* For RHEL/Rocky Linux/Alma Linux, use `dnf install /path/to/percona-proxy-*.rpm`

* For Debian and Ubuntu, use `apt install /path/to/percona-proxy-*.deb`.

* For SLES, use `zypper install /path/to/percona-proxy-*.rpm`.

## Install Percona Proxy for MariaDB Using a Tarball

Percona Proxy can also be installed using a tarball.
That may be required if you are using a Linux distribution for which there
exist no installation package or if you want to install many different
Percona Proxy versions side by side. For instructions on how to do that, please refer to
[Install Percona Proxy for MariaDB using a Tarball](Install-MariaDB-Percona-Proxy-Using-a-Tarball.md).

## Building Percona Proxy for MariaDB From Source Code

Alternatively you may download the Percona Proxy for MariaDB source and build your own binaries.
To do this, refer to the separate document
[Building Percona Proxy for MariaDB from Source Code](Building-Percona-Proxy-from-Source-Code.md)

## Assumptions

### Memory allocation behavior

Percona Proxy assumes that memory allocations always succeed and in general does
not check for memory allocation failures. This assumption is compatible with
the Linux kernel parameter
[`vm.overcommit_memory`](https://www.kernel.org/doc/Documentation/vm/overcommit-accounting)
having the value `0`, which is also the default on most systems.

With `vm.overcommit_memory` being `0`, memory _allocations_ made by an
application never fail, but instead the application may be killed by the
so-called OOM (out-of-memory) killer if, by the time the application
actually attempts to _use_ the allocated memory, there is not available
free memory on the system.

If the value is `2`, then a memory allocation made by an application may
fail and unless the application is prepared for that possiblity, it will
likely crash with a SIGSEGV. As Percona Proxy is not prepared to handle memory
allocation failures, it will crash in this situation.

The current value of `vm.overcommit_memory` can be checked with
```
sysctl vm.overcommit_memory
```
or
```
cat /proc/sys/vm/overcommit_memory
```

## Configuring Percona Proxy for MariaDB

[The Percona Proxy Tutorial](../Tutorials/Percona-Proxy-Tutorial.md) covers the first
steps in configuring your Percona Proxy for MariaDB installation. Follow this tutorial
to learn how to configure and start using Percona Proxy.

For a detailed list of all configuration parameters, refer to the
[Configuration Guide](Configuration-Guide.md) and the module specific documents
listed in the [Documentation Contents](../Documentation-Contents.md#routers).

## Encrypting Passwords

Read the [Encrypting Passwords](Configuration-Guide.md#encrypting-passwords)
section of the configuration guide to set up password encryption for the
configuration file.

## Administration Of Percona Proxy for MariaDB

There are various administration tasks that may be done with Percona Proxy for MariaDB.
A command line tools is available, [percona-proxyctl](../Reference/Percona-Proxyctl.md), that will
interact with a running Percona Proxy for MariaDB and allow the status of MariaDB
Percona Proxy to be monitored and give some control of the Percona Proxy for MariaDB
functionality.

[The administration tutorial](../Tutorials/Administration-Tutorial.md)
covers the common administration tasks that need to be done with Percona Proxy for MariaDB.

## Copying or Backing Up Percona Proxy

The main configuration file for Percona Proxy is in `/etc/percona-proxy.cnf` and
additional user-created configuration files are in
`/etc/percona-proxy.cnf.d/`. Objects created or modified at runtime are stored in
`/var/lib/percona-proxy/percona-proxy.cnf.d/`. Some modules also store internal data in
`/var/lib/percona-proxy/` named after the module or the configuration object.

The simplest way to back up the configuration and runtime data of a Percona Proxy
installation is to create an archive from the following files and directories:

* `/etc/percona-proxy.cnf`

* `/etc/percona-proxy.cnf.d/`

* `/var/lib/percona-proxy/`

This can be done with the following command:

```
tar -caf percona-proxy-backup.tar.gz /etc/percona-proxy.cnf /etc/percona-proxy.cnf.d/ /var/lib/percona-proxy/
```

If Percona Proxy is configured to store data in custom locations, these should be
included in the backup as well.
