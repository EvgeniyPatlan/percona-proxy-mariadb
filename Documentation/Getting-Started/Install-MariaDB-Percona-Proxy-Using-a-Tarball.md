# Installing Percona Proxy for MariaDB using a tarball

Percona Proxy for MariaDB is also made available as a tarball, which is named like
`percona-proxy-x.y.z.OS.tar.gz` where `x.y.z` is the same as the corresponding version and `OS`
identifies the operating system, e.g. `percona-proxy-2.5.6.centos.7.tar.gz`.

In order to use the tarball, the following libraries are required:

- libcurl
- libaio
- OpenSSL
- gnutls
- libatomic
- unixODBC

The tarball has been built with the assumption that it will be installed in `/usr/local`.
However, it is possible to install it in any directory, but in that case Percona Proxy for MariaDB
must be invoked with a flag.

## Installing as root in `/usr/local`

If you have root access to the system you probably want to install Percona Proxy for MariaDB under
the user and group `percona-proxy`.

The required steps are as follows:

    $ sudo groupadd percona-proxy
    $ sudo useradd -g percona-proxy percona-proxy
    $ cd /usr/local
    $ sudo tar -xzvf percona-proxy-x.y.z.OS.tar.gz
    $ sudo ln -s percona-proxy-x.y.z.OS percona-proxy
    $ cd percona-proxy
    $ sudo chown -R percona-proxy var

Creating the symbolic link is necessary, since Percona Proxy for MariaDB has been built
with the assumption that the plugin directory is `/usr/local/percona-proxy/lib/percona-proxy`.

The symbolic link also makes it easy to switch between different versions of
Percona Proxy for MariaDB that have been installed side by side in `/usr/local`;
just make the symbolic link point to another installation.

In addition, the first time you install Percona Proxy for MariaDB from a tarball
you need to create the following directories:

    $ sudo mkdir /var/log/percona-proxy
    $ sudo mkdir /var/lib/percona-proxy
    $ sudo mkdir /var/run/percona-proxy
    $ sudo mkdir /var/cache/percona-proxy

and make `percona-proxy` the owner of them:

    $ sudo chown percona-proxy /var/log/percona-proxy
    $ sudo chown percona-proxy /var/lib/percona-proxy
    $ sudo chown percona-proxy /var/run/percona-proxy
    $ sudo chown percona-proxy /var/cache/percona-proxy

The following step is to create the Percona Proxy for MariaDB configuration file `/etc/percona-proxy.cnf`.
The file `etc/percona-proxy.cnf.template` can be used as a base.
Please refer to [Configuration Guide](Configuration-Guide.md) for details.

When the configuration file has been created, Percona Proxy for MariaDB can be started.

    $ sudo bin/percona-proxy --user=percona-proxy -d

The `-d` flag causes percona-proxy _not_ to turn itself into a daemon,
which is adviseable the first time Percona Proxy for MariaDB is started, as it makes it easier to spot problems.

If you want to place the configuration file somewhere else but in `/etc`
you can invoke Percona Proxy for MariaDB with the `--config` flag,
for instance, `--config=/usr/local/percona-proxy/etc/percona-proxy.cnf`.

Note also that if you want to keep _everything_ under `/usr/local/percona-proxy`
you can invoke Percona Proxy for MariaDB using the flag `--basedir`.

    $ sudo bin/percona-proxy --user=percona-proxy --basedir=/usr/local/percona-proxy -d

That will cause Percona Proxy for MariaDB to look for its configuration file in
`/usr/local/percona-proxy/etc` and to store all runtime files under `/usr/local/percona-proxy/var`.

## Installing in any Directory

Enter a directory where you have the right to create a subdirectory. Then do as follows.

    $ tar -xzvf percona-proxy-x.y.z.OS.tar.gz

The next step is to create the Percona Proxy configuration file `percona-proxy-x.y.z/etc/percona-proxy.cnf`.
The file `percona-proxy-x.y.z/etc/percona-proxy.cnf.template` can be used as a base.
Please refer to [Configuration Guide](Configuration-Guide.md) for details.

When the configuration file has been created, Percona Proxy for MariaDB can be started.

    $ cd percona-proxy-x.y.z.OS
    $ bin/percona-proxy -d --basedir=.

With the flag `--basedir`, Percona Proxy for MariaDB is told where the `lib`, `etc` and `var`
directories are found. Unless it is specified, Percona Proxy for MariaDB assumes
the `lib` directory is found in `/usr/local/percona-proxy`,
and the `var` and `etc` directories in `/`.

It is also possible to specify the directories and the location of
the configuration file individually. Invoke Percona Proxy like

    $ bin/percona-proxy --help

to find out the appropriate flags.
