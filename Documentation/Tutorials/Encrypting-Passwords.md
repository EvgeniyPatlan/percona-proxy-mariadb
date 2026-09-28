# Encrypting Passwords

**Note**: The password encryption format changed in Percona Proxy 2.5. All
  encrypted passwords created with Percona Proxy 2.4 or older need to be
  re-encrypted.

There are two options for representing the password, either plain text or
encrypted passwords may be used. In order to use encrypted passwords a set of
keys must be generated that will be used by the encryption and decryption
process. To generate the keys, use the `percona-proxy-keys` command.

```
percona-proxy-keys
```

By default the key file will be generated in `/var/lib/percona-proxy`. If a different
directory is required, it can be given as the first argument to the program. For
more information, see `percona-proxy-keys --help`.

Once the keys have been created the `percona-proxy-passwd` command can be used to generate
the encrypted password.

```
percona-proxy-passwd plainpassword
96F99AA1315BDC3604B006F427DD9484
```

The username and password, either encrypted or plain text, are stored in the
service section using the `user` and `password` parameters.

If a custom location was used for the key file, give it as the first argument to
`percona-proxy-passwd` and pass the password to be encrypted as the second argument. For
more information, see `percona-proxy-keys --help`.

Here is an example configuration that uses an encrypted password.

```
[My-Service]
type=service
router=readconnroute
router_options=master
servers=dbserv1, dbserv2, dbserv3
user=percona-proxy
password=96F99AA1315BDC3604B006F427DD9484
```

If the key file is not in the default location, the
[`datadir`](../Getting-Started/Configuration-Guide.md#datadir) parameter must be
set to the directory that contains it.
