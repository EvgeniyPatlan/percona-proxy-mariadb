# Upgrading to Percona Proxy for MariaDB

Percona Proxy for MariaDB 1.0.0 is derived from MariaDB MaxScale 23.08.12. Moving to it from
that release changes names, not behaviour: the configuration syntax, the modules and their
parameters are the same.

Back up the configuration before starting.

[TOC]

# From MariaDB MaxScale 23.08 to Percona Proxy for MariaDB 1.0.0

Percona Proxy does not replace the `maxscale` package. Every installed path, the service and
the system user carry the new name, so both can be installed while a deployment is moved over,
and the move can be undone by starting `maxscale` again.

1. Install `percona-proxy-mariadb`.

2. Convert the configuration:

   ```
   percona-proxy-migrate --dry-run   # show what would change
   percona-proxy-migrate             # /etc/maxscale.cnf -> /etc/percona-proxy.cnf
   ```

   Add `--data` to copy the data directory and the persisted configuration that the REST API
   writes. Nothing is removed from the MaxScale installation.

3. Check `/etc/percona-proxy.cnf`. Paths that the packages do not own, such as a `filebase`
   under a directory of your own, keep the name they had.

4. Stop MaxScale and start Percona Proxy:

   ```
   systemctl stop maxscale
   systemctl enable --now percona-proxy
   percona-proxyctl list servers
   ```

Until the configuration is converted, `/etc/maxscale.cnf` is read when `/etc/percona-proxy.cnf`
does not exist, and a global section still called `[maxscale]` is accepted. Both log a warning,
and both are migration aids rather than a supported configuration.

## What changed

The names are listed in the
[1.0.0 release notes](../Release-Notes/Percona-Proxy-1.0.0-Release-Notes.md). The ones that
affect something other than a file path:

* The session variables the cache and LDI filters register are `@percona_proxy.*`, with an
  underscore, because a hyphen is not valid in an SQL variable name.
* The REST API is served under `/v1/percona-proxy`, and the client is `percona-proxyctl`.
* `percona-proxyctl` reads `PERCONA_PROXYCTL_USER`, `PERCONA_PROXYCTL_PASSWORD` and
  `PERCONA_PROXYCTL_WARNINGS`.

# Earlier upgrades

[Upgrading MaxScale](Upgrading-MaxScale.md) covers the upgrades between MariaDB MaxScale
releases that came before this product. Those sections keep the upstream name, because they
describe changes MariaDB made.
