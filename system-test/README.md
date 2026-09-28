# percona-proxy-system-test

System level tests for Percona Proxy

## Basics

- Every test is separate executable file

- Environment variables contain all information about backends: IPs, user names,
  passwords etc.

- Backends are created with the help of
  [MDBCI](https://github.com/mariadb-corporation/mdbci) and are defined by the
  labels in the tests.

  - `REPL_BACKEND` creates a 4 node async replication cluster

  - `GALERA_BACKEND` creates a 4 node Galera cluster

  - `SECOND_MAXSCALE` creates a second Percona Proxy node (`percona_proxy_001`)
