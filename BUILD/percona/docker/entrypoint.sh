#!/bin/sh
#
# Entrypoint of the Percona Proxy for MariaDB image.
#
# Percona Proxy reads /etc/percona-proxy.cnf, which the image ships with a minimal configuration and which
# is meant to be replaced by a mounted one. Files in /etc/percona-proxy.cnf.d are read as well.
#
set -o errexit

if [ "$1" = "percona-proxy" ]
then
    shift

    # Directories can come from volumes that are empty or owned by another user.
    for dir in /var/lib/percona-proxy /var/log/percona-proxy /var/cache/percona-proxy /run/percona-proxy
    do
        if [ ! -w "$dir" ]
        then
            echo >&2 "ERROR: $dir is not writable by the percona-proxy user ($(id -u):$(id -g))."
            echo >&2 "       Mount it with those permissions, for example: -v percona-proxy-data:/var/lib/percona-proxy"
            exit 1
        fi
    done

    if [ ! -r /etc/percona-proxy.cnf ]
    then
        echo >&2 "ERROR: /etc/percona-proxy.cnf is missing or not readable."
        exit 1
    fi

    # --nodaemon keeps Percona Proxy in the foreground so that the container follows its lifetime,
    # and the log goes to stdout for "docker logs".
    exec percona-proxy --nodaemon --log=stdout "$@"
fi

exec "$@"
