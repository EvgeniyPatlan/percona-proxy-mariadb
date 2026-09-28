# Percona Proxy version for CMake
#
# This file contains cache values for CMake which control Percona Proxy's version
# number.

set(PERCONA_PROXY_VERSION_MAJOR "21" CACHE STRING "Major version" FORCE)
set(PERCONA_PROXY_VERSION_MINOR "06" CACHE STRING "Minor version" FORCE)
set(PERCONA_PROXY_VERSION_PATCH "22" CACHE STRING "Patch version" FORCE)

# Used in version.hh.in, no leading 0.
set(PERCONA_PROXY_VERSION_MINOR_NUM "6" CACHE STRING "Minor version")

# This should only be incremented if a package is rebuilt
set(PERCONA_PROXY_BUILD_NUMBER 1 CACHE STRING "Release number")

set(PERCONA_PROXY_MATURITY "GA" CACHE STRING "Release maturity")

set(PERCONA_PROXY_VERSION_NUMERIC "${PERCONA_PROXY_VERSION_MAJOR}.${PERCONA_PROXY_VERSION_MINOR}.${PERCONA_PROXY_VERSION_PATCH}")
set(PERCONA_PROXY_VERSION "${PERCONA_PROXY_VERSION_MAJOR}.${PERCONA_PROXY_VERSION_MINOR}.${PERCONA_PROXY_VERSION_PATCH}")
