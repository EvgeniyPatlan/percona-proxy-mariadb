#
# Spec file for Percona Proxy for MariaDB.
#
# @@VERSION@@ and @@RELEASE@@ are replaced by BUILD/percona/percona_proxy_builder.sh before
# the source RPM is built.
#
# Node.js (>= 16) and npm are needed to build Percona Proxyctl and the GUI. They are
# intentionally not listed in BuildRequires because the distribution packages are frequently
# too old; install them with BUILD/install_build_deps.sh, which fetches a suitable version.
#
# The build needs network access: FORCE_BUNDLE (on by default) downloads and statically builds
# jansson, microhttpd, pcre2, libssh and rdkafka, the LDI filter fetches libmarias3, and
# Percona Proxyctl and the GUI install their npm dependencies.
#

# The bundled libmicrohttpd does not build with the link-time optimization that RHEL 10 and
# later put into the default build flags: it warns, and the build uses -Werror.
%global _lto_cflags %{nil}
# Ship the binaries unstripped and the man pages uncompressed, like the CPack packages did.
# This also disables the debuginfo subpackage.
%global __os_install_post %{nil}
%global debug_package %{nil}

Name:           percona-proxy-mariadb
Version:        @@VERSION@@
Release:        @@RELEASE@@%{?dist}
Summary:        Percona Proxy for MariaDB - an intelligent database proxy

# Percona Proxy for MariaDB is derived from MariaDB MaxScale 23.08, which was released under
# the Business Source License 1.1 with the Change Date 2026-09-21. That date has passed, so
# this code is governed by the Change License, version 2 or later of the GNU General Public
# License. The original BSL text ships as LICENSE.TXT.
License:        GPL-2.0-or-later
URL:            https://github.com/EvgeniyPatlan/percona-proxy-mariadb
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  git
BuildRequires:  bison
BuildRequires:  flex
BuildRequires:  pkgconfig
BuildRequires:  boost-devel
BuildRequires:  cyrus-sasl-devel
BuildRequires:  gnutls-devel
BuildRequires:  krb5-devel
BuildRequires:  libatomic
BuildRequires:  libcurl-devel
BuildRequires:  libgcrypt-devel
BuildRequires:  libicu-devel
BuildRequires:  libssh-devel
BuildRequires:  libuuid-devel
BuildRequires:  libxml2-devel
BuildRequires:  openssl-devel
BuildRequires:  pam-devel
BuildRequires:  pcre2-devel
BuildRequires:  jansson-devel
BuildRequires:  sqlite-devel
BuildRequires:  systemd-devel
BuildRequires:  tcl
BuildRequires:  unixODBC-devel
BuildRequires:  xz-devel
BuildRequires:  zlib-devel

Requires(post): systemd
Requires(post): shadow-utils
Requires(preun): systemd

# No Provides, Conflicts or Obsoletes against maxscale: this is a separate product, not a
# drop-in replacement for that package. Every installed path, the service and the system user
# carry the percona-proxy name, so the two can be installed side by side while a deployment is
# migrated with percona-proxy-migrate.

%description
Percona Proxy for MariaDB is an intelligent proxy that allows forwarding of
database statements to one or more database servers using complex rules,
a semantic understanding of the database statements and the roles of
the various servers within the backend cluster of databases.
It is designed to provide load balancing and high availability
functionality transparently to the applications. In addition it provides
a highly scalable and flexible architecture, with plugin components to
support different protocols and routing decisions.

%package devel
Summary:        Percona Proxy for MariaDB plugin development headers
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
This package contains header files required for plugin module development for
Percona Proxy for MariaDB. The source of Percona Proxy for MariaDB is not required.

%prep
%autosetup -n %{name}-%{version}

%build
mkdir -p build
cd build
# The tarball has no .git directory; the commit ID travels in percona-build.properties.
percona_proxy_commit=$(sed -n 's/^COMMIT=//p' ../percona-build.properties 2>/dev/null)
# PACKAGE=Y selects the packaging layout: prefix /usr, and the systemd unit, ld.so
# configuration and init script are installed below %{_datadir}/percona-proxy for the
# postinst script to place, exactly as in a CPack build.
cmake .. \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_COLOR_MAKEFILE=N \
    -DPACKAGE=Y \
    -DPACKAGE_NAME=%{name} \
    -DTARGET_COMPONENT=core,devel \
    -DSKIP_CPACK=Y \
    -DBUILD_TESTS=N \
    -DPERCONA_PROXY_COMMIT="${percona_proxy_commit}" \
    %{?extra_cmake_flags}
make %{?_smp_mflags}

%install
cd build
make install DESTDIR=%{buildroot}
cd ..

# CMake installs this only when TARGET_COMPONENT is exactly "core", so do it here.
install -D -m 0644 etc/percona-proxy.cnf.template \
    %{buildroot}%{_sysconfdir}/percona-proxy.cnf.template

# The Change License text; RPM-based distributions ship no shared copy of it.
install -D -m 0644 BUILD/percona/packaging/licenses/GPL-2.0.txt \
    %{buildroot}%{_datadir}/percona-proxy/GPL-2.0.txt

%post
sh %{_datadir}/percona-proxy/postinst

%preun
sh %{_datadir}/percona-proxy/prerm "$1"

%files
%{_sysconfdir}/percona-proxy.cnf.template
%{_bindir}/*
%{_libdir}/percona-proxy/
%{_datadir}/percona-proxy/
%{_mandir}/man1/*

%files devel
%{_includedir}/percona-proxy/

%changelog
* Mon Sep 28 2026 Percona Build Team <info@percona.com> - @@VERSION@@-@@RELEASE@@
- Percona Proxy for MariaDB @@VERSION@@, derived from MariaDB MaxScale 23.08.12
