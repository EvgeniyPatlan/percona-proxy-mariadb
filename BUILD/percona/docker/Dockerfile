FROM redhat/ubi9-minimal

# Please don't remove old-style LABEL since it's needed for RedHat certification
LABEL name="Percona Proxy for MariaDB" \
      vendor="Percona" \
      summary="Database proxy for MariaDB and MySQL" \
      description="Percona Proxy for MariaDB is a database proxy that forwards database statements to one or more servers, using complex rules, a semantic understanding of the statements and the roles of the servers in the backend cluster. It provides load balancing and high availability transparently to the applications." \
      maintainer="Percona Development <info@percona.com>"

LABEL org.opencontainers.image.title="Percona Proxy for MariaDB"
LABEL org.opencontainers.image.vendor="Percona"
LABEL org.opencontainers.image.description="Percona Proxy for MariaDB is a database proxy that forwards \
database statements to one or more servers, using complex rules, a semantic understanding of the \
statements and the roles of the servers in the backend cluster."
LABEL org.opencontainers.image.license="GPL-2.0-or-later"
LABEL org.opencontainers.image.authors="info@percona.com"

# The component of repo.percona.com to install from, and the version to pin to.
ARG PERCONA_PROXY_VERSION=1.0.0
ARG PERCONA_PROXY_RELEASE=1
ARG REPO_COMPONENT=experimental

ENV PERCONA_PROXY_VERSION=${PERCONA_PROXY_VERSION}
LABEL org.label-schema.schema-version=${PERCONA_PROXY_VERSION}
LABEL org.opencontainers.image.version=${PERCONA_PROXY_VERSION}

# Check the repository package signature in a secure way.
RUN set -ex; \
    export GNUPGHOME="$(mktemp -d)"; \
    microdnf install -y findutils gnupg2 tar; \
    gpg --batch --keyserver keyserver.ubuntu.com --recv-keys 4D1BB29D63D98E422B2113B19334A25F8507EFA5; \
    gpg --batch --export --armor 4D1BB29D63D98E422B2113B19334A25F8507EFA5 > ${GNUPGHOME}/PERCONA-PACKAGING-KEY; \
    rpmkeys --import ${GNUPGHOME}/PERCONA-PACKAGING-KEY; \
    curl -Lf -o /tmp/percona-release.rpm https://repo.percona.com/yum/percona-release-latest.noarch.rpm; \
    rpmkeys --checksig /tmp/percona-release.rpm; \
    rpm -i /tmp/percona-release.rpm; \
    rm -rf "$GNUPGHOME" /tmp/percona-release.rpm; \
    rpm --import /etc/pki/rpm-gpg/PERCONA-PACKAGING-KEY; \
    percona-release enable percona-proxy ${REPO_COMPONENT}

RUN set -ex; \
    microdnf install -y \
        percona-proxy-mariadb-${PERCONA_PROXY_VERSION}-${PERCONA_PROXY_RELEASE}.el9 \
        shadow-utils \
        procps-ng \
        hostname; \
    microdnf update -y glibc openssl-libs; \
    microdnf clean all; \
    rm -rf /var/cache/dnf /var/cache/yum

# Percona Proxy runs as this user; the packages create it, so only the ownership is set here.
RUN set -ex; \
    install -o percona-proxy -g percona-proxy -m 0755 -d \
        /var/lib/percona-proxy /var/log/percona-proxy /var/cache/percona-proxy /run/percona-proxy \
        /var/lib/percona-proxy/percona-proxy.cnf.d; \
    mkdir -p /licenses; \
    cp /usr/share/percona-proxy/LICENSE.TXT /licenses/LICENSE.percona-proxy; \
    cp /usr/share/percona-proxy/GPL-2.0.txt /licenses/GPL-2.0.txt

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
COPY percona-proxy.cnf /etc/percona-proxy.cnf
RUN chmod +x /usr/local/bin/entrypoint.sh; \
    chown percona-proxy:percona-proxy /etc/percona-proxy.cnf

# readwritesplit, readconnroute and the REST API / GUI
EXPOSE 4006 4008 8989

VOLUME ["/var/lib/percona-proxy", "/var/log/percona-proxy"]

HEALTHCHECK --interval=10s --timeout=5s --start-period=30s --retries=3 \
    CMD percona-proxyctl list servers > /dev/null || exit 1

USER percona-proxy
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["percona-proxy"]
