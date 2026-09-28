# Percona Proxy for MariaDB version for CMake
#
# This file contains cache values for CMake which control the version number of
# Percona Proxy for MariaDB.
#
# The product is versioned independently of the MariaDB MaxScale release it is derived from.
# The upstream numbers stay in VERSION2308.cmake, which is kept for provenance and is no
# longer included by VERSION.cmake.

set(PERCONA_PROXY_VERSION_MAJOR "1" CACHE STRING "Major version")
set(PERCONA_PROXY_VERSION_MINOR "0" CACHE STRING "Minor version")
set(PERCONA_PROXY_VERSION_PATCH "0" CACHE STRING "Patch version")

# Used in version.hh.in, no leading 0.
set(PERCONA_PROXY_VERSION_MINOR_NUM "0" CACHE STRING "Minor version")

# This should only be incremented if a package is rebuilt
set(PERCONA_PROXY_BUILD_NUMBER 1 CACHE STRING "Release number")

set(PERCONA_PROXY_MATURITY "GA" CACHE STRING "Release maturity")

set(PERCONA_PROXY_VERSION "${PERCONA_PROXY_VERSION_MAJOR}.${PERCONA_PROXY_VERSION_MINOR}.${PERCONA_PROXY_VERSION_PATCH}")

# The MariaDB MaxScale release this code is derived from, reported alongside the product
# version so a build can always be traced back to its upstream base.
set(PERCONA_PROXY_UPSTREAM_VERSION "23.08.12" CACHE STRING "Upstream MaxScale version")
