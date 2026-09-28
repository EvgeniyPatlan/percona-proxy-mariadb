# Percona Proxy for MariaDB 1.0.0 Release Notes

Percona Proxy for MariaDB 1.0.0 is the first release. It is derived from MariaDB MaxScale
23.08.12, and carries its functionality unchanged; what differs is the name.

## Changes from MariaDB MaxScale 23.08.12

### Names

| MariaDB MaxScale | Percona Proxy for MariaDB |
|------------------|---------------------------|
| `maxscale` | `percona-proxy` |
| `maxctrl` | `percona-proxyctl` |
| `maxkeys`, `maxpasswd`, `maxavrocheck` | `percona-proxy-keys`, `percona-proxy-passwd`, `percona-proxy-avrocheck` |
| `/etc/maxscale.cnf` | `/etc/percona-proxy.cnf` |
| `[maxscale]` | `[percona-proxy]` |
| `/var/lib/maxscale`, `/var/log/maxscale`, `/var/cache/maxscale` | the same paths under `percona-proxy` |
| `maxscale` user and service | `percona-proxy` user and service |
| `maxscale` package | `percona-proxy-mariadb` package |
| `@maxscale.cache.*`, `@maxscale.ldi.*` session variables | `@percona_proxy.cache.*`, `@percona_proxy.ldi.*` |
| `MAXCTRL_USER`, `MAXCTRL_PASSWORD`, `MAXCTRL_WARNINGS` | `PERCONA_PROXYCTL_USER`, `PERCONA_PROXYCTL_PASSWORD`, `PERCONA_PROXYCTL_WARNINGS` |
| `/v1/maxscale` REST path | `/v1/percona-proxy` |

Module names are unchanged, so `readwritesplit`, `mariadbmon` and the rest are configured
exactly as before.

### Migrating an existing installation

`percona-proxy-migrate` converts a MaxScale configuration into a Percona Proxy one. It rewrites
the section name and the paths the packages own, leaves everything else alone, and removes
nothing, so the MaxScale installation is still there afterwards.

```
percona-proxy-migrate --dry-run     # show what would change
percona-proxy-migrate               # /etc/maxscale.cnf -> /etc/percona-proxy.cnf
percona-proxy-migrate --data        # also copy the data and the persisted configuration
```

Until a configuration is migrated, `/etc/maxscale.cnf` is read when `/etc/percona-proxy.cnf`
does not exist, and a global section still called `[maxscale]` is accepted. Both log a warning.

Percona Proxy for MariaDB does not replace the `maxscale` package: the paths, the service and
the system user differ, so both can be installed while a deployment is moved over.

### Licence

MariaDB MaxScale 23.08 was published under the Business Source License 1.1 with the Change Date
2026-09-21 and version 2 or later of the GNU General Public License as the Change License. That
date has passed, so this code is available under the GPL. See `COPYING` and `NOTICE`.
