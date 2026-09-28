# Authentication Modules

This document describes general MySQL protocol authentication in Percona Proxy. For
REST-api authentication, see the
[configuration guide](../Getting-Started/Configuration-Guide.md) and the
[REST-api guide](../REST-API/API.md).

Similar to the MariaDB Server, Percona Proxy uses authentication plugins to implement
different authentication schemes for incoming clients. The same plugins also
handle authenticating the clients to backend servers. The authentication plugins
available in Percona Proxy are
[standard MySQL password](MySQL-Authenticator.md),
[GSSAPI](GSSAPI-Authenticator.md) and
[pluggable authentication modules (PAM)](PAM-Authenticator.md).

Most of the authentication processing is performed on the protocol level, before
handing it over to one of the plugins. This shared part is described in this
document. For information on an individual plugin, see its documentation.

## User account management

Every Percona Proxy service with a MariaDB protocol listener requires knowledge of
the user accounts defined on the backend databases. The service maintains this
information in an internal component called the *user account manager* (UAM).
The UAM queries relevant data from the *mysql*-database of the backends and
stores it. Typically, only the current primary server is queried, as all servers
are assumed to have the same users. The service settings *user* and *password*
define the credentials used when fetching user accounts.

The service uses the stored data when authenticating clients, checking their
passwords and database access rights. This results in an authentication process
very similar to the MariaDB Server itself. Unauthorized users are generally
detected already at the Percona Proxy level instead of the backend servers. This may
not apply in some cases, for example if Percona Proxy is using old user account data.

If authentication fails, the UAM updates its data from a backend. Percona Proxy may
attempt authenticating the client again with the refreshed data without
communicating the first failure to the client. This transparent user data update
does not always work, in which case the client should try to log in again.

As the UAM is shared between all listeners of a service, its settings are
defined in the service configuration. For more information, search the
[configuration guide](../Getting-Started/Configuration-Guide.md)
for *users_refresh_time*, *users_refresh_interval* and
*auth_all_servers*. Other settings which affect how the UAM connects to backends
are the global settings *auth_connect_timeout* and *local_address*, and
the various server-level ssl-settings.

### Required grants

To properly fetch user account information, the Percona Proxy service user must be
able to read from various tables in the  *mysql*-database: *user*, *db*,
*tables_priv*, *columns_priv*, *procs_priv*, *proxies_priv* and *roles_mapping*.
The user should also have the *SHOW DATABASES*-grant.

```
CREATE USER 'percona-proxy'@'percona_proxy_host' IDENTIFIED BY 'percona-proxy-password';
GRANT SELECT ON mysql.user TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.db TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.tables_priv TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.columns_priv TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.procs_priv TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.proxies_priv TO 'percona-proxy'@'percona_proxy_host';
GRANT SELECT ON mysql.roles_mapping TO 'percona-proxy'@'percona_proxy_host';
GRANT SHOW DATABASES ON *.* TO 'percona-proxy'@'percona_proxy_host';
```

If using MariaDB ColumnStore, the following grant is required:

```
GRANT ALL ON infinidb_vtable.* TO 'percona-proxy'@'percona_proxy_host';
```

## Limitations and troubleshooting

When a client logs in to Percona Proxy, Percona Proxy sees the client's IP address. When
Percona Proxy then connects the client to backends (using the client's username and
password), the backends see the connection coming from the IP address of
Percona Proxy. If the client user account is to a wildcard host (`'alice'@'%'`), this
is not an issue. If the host is restricted (`'alice'@'123.123.123.123'`),
authentication to backends will fail.

There are two primary ways to deal with this:
1. Duplicate user accounts. For every user account with a restricted hostname an
equivalent user account for Percona Proxy is added (`'alice'@'percona-proxy-ip'`).
2. Use [proxy protocol](../Getting-Started/Configuration-Guide.md#proxy_protocol).

Option 1 limits the passwords for user accounts with shared usernames. Such
accounts must use the same password since they will effectively share the
Percona Proxy-to-backend user account. Option 2 requires server support.

See
[Percona Proxy Troubleshooting](https://mariadb.com/kb/en/mariadb-enterprise/percona-proxy-troubleshooting/)
for additional information on how to solve authentication issues.

### Wildcard database grants

Percona Proxy supports wildcards `_` and `%` for database-level grants. As with
MariaDB Server, `grant select on test_.* to 'alice'@'%';` gives access to
*test_* as well as *test1*, *test2* and so on. If the GRANT command escapes the
wildcard (``grant select on `test\_`.* to 'alice'@'%';``) both Percona Proxy and the
MariaDB Server interpret it as only allowing access to *test_*. `_` and `%`
are only interpreted as wildcards when the grant is to a database:
``grant select on `test_`.t1 to 'alice'@'%';`` only grants access to the
*test_.t1*-table, not to *test1.t1*.

## Authenticator options

The listener configuration defines authentication options which only affect the
listener. *authenticator* defines the authentication plugins to use.
*authenticator_options* sets various options. These options may affect an
individual authentication plugin or the authentication as a whole. The latter
are explained below. Multiple options can be given as a comma-separated list.

```
authenticator_options=skip_authentication=true,lower_case_table_names=1
```

### `skip_authentication`

- **Type**: [boolean](../Getting-Started/Configuration-Guide.md#booleans)
- **Mandatory**: No
- **Dynamic**: No
- **Default**: `false`

If enabled, Percona Proxy will not check the
passwords of incoming clients and just assumes that they are correct.
Wrong passwords are instead detected when Percona Proxy tries to authenticate to the
backend servers.

This setting is mainly meant for failure tolerance in situations where the
password check is performed outside of Percona Proxy. If, for example, Percona Proxy
cannot use an LDAP-server but the backend databases can, enabling this setting
allows clients to log in. Even with this setting enabled, a user account
matching the incoming client username and IP must exist on the backends for
Percona Proxy to accept the client.

This setting is incompatible with standard MariaDB/MySQL authentication plugin
(*MariaDBAuth* in Percona Proxy). If enabled, Percona Proxy cannot authenticate clients to
backend servers using standard authentication.

```
authenticator_options=skip_authentication=true
```

### `match_host`

- **Type**: [boolean](../Getting-Started/Configuration-Guide.md#booleans)
- **Mandatory**: No
- **Dynamic**: No
- **Default**: `true`

If disabled, Percona Proxy does not require that a
valid user account entry for incoming clients exists on the backends.
Specifically, only the client username needs to match a user account,
hostname/IP is ignored.

This setting may be used to force clients to connect through Percona Proxy. Normally,
creating the user *jdoe@%* will allow the user *jdoe* to connect from any
IP-address. By disabling *match_host* and replacing the user with
*jdoe@percona-proxy-IP*, the user can still connect from any client IP but will be
forced to go through Percona Proxy.

```
authenticator_options=match_host=false
```

### `lower_case_table_names`

- **Type**: number
- **Mandatory**: No
- **Dynamic**: No
- **Default**: `0`

Controls database name matching for authentication
when an incoming client logs in to a non-empty database. The setting functions
similar to the MariaDB Server setting
[lower_case_table_names](https://mariadb.com/kb/en/library/server-system-variables/#lower_case_table_names)
and should be set to the value used by the backends.

The setting accepts the values 0, 1 or 2:

* `0`: case-sensitive matching (default)
* `1`: convert the requested database name to lower case before using case-insensitive
matching. Assumes that database names on the server are stored in lower case.
* `2`: use case-insensitive matching.

*true* and *false* are also accepted for backwards compatibility. These map to 1
and 0, respectively.

The identifier names are converted using an ASCII-only function. This means that
non-ASCII characters will retain their case-sensitivity.

Starting with Percona Proxy versions 2.5.25, 6.4.6, 22.08.5 and 23.02.2, the behavior
of `lower_case_table_names=1` is identical with how the MariaDB server
behaves. In older releases the comparisons were done in a case-sensitive manner
after the requested database name was converted into lowercase. Using
`lower_case_table_names=2` will behave identically in all versions which makes
it a safe alternative to use when a mix of older and newer Percona Proxy versions is
being used.

```
authenticator_options=lower_case_table_names=0
```
