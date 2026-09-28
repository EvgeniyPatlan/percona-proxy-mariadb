# Percona Proxy version for CMake
#
# This file contains cache values for CMake which control Percona Proxy's version
# number.

set(PERCONA_PROXY_VERSION_MAJOR "99" CACHE STRING "Major version")
set(PERCONA_PROXY_VERSION_MINOR "99" CACHE STRING "Minor version")
set(PERCONA_PROXY_VERSION_PATCH "99" CACHE STRING "Patch version")

# Used in version.hh.in, no leading 0.
set(PERCONA_PROXY_VERSION_MINOR_NUM "99" CACHE STRING "Minor version")

# This should only be incremented if a package is rebuilt
set(PERCONA_PROXY_BUILD_NUMBER 1 CACHE STRING "Release number")

# The maturity of 'develop' is by definition "Develop"; it is never anything else.
set(PERCONA_PROXY_MATURITY "Develop" CACHE STRING "Release maturity")

set(PERCONA_PROXY_VERSION "${PERCONA_PROXY_VERSION_MAJOR}.${PERCONA_PROXY_VERSION_MINOR}.${PERCONA_PROXY_VERSION_PATCH}")
