#
# Builds libmemcached: https://libmemcached.org/libMemcached.html
#
# Sources taken from https://launchpad.net/libmemcached/+download
#
# The following variables are set:
# LIBMEMCACHED_VERSION     - The libcached version used.
# LIBMEMCACHED_URL         - The download URL.
# LIBMEMCACHED_INCLUDE_DIR - The include directories
# LIBMEMCACHED_LIBRARIES   - The libraries to link
# LIBMEMCACHED_FOUND       - Always true
#

set(LIBMEMCACHED_VERSION "1.0.18")

message(STATUS "Building libmemcached version ${LIBMEMCACHED_VERSION}")

set(LIBMEMCACHED_URL "https://launchpad.net/libmemcached/1.0/${LIBMEMCACHED_VERSION}/+download/libmemcached-${LIBMEMCACHED_VERSION}.tar.gz"
  CACHE STRING "libmemcached sources")

# Launchpad is the only place upstream fetches this from, and it has answered 502 and 503
# during builds more than once, which fails the whole package build. CMake tries the URLs in
# turn until one succeeds, and URL_HASH below is checked whichever one answers, so a mirror
# cannot change what is built. The MacPorts copy is byte identical.
set(LIBMEMCACHED_URL_MIRROR "https://distfiles.macports.org/libmemcached/libmemcached-${LIBMEMCACHED_VERSION}.tar.gz"
  CACHE STRING "libmemcached sources, used when the first URL cannot be reached")

set(LIBMEMCACHED_BASE "${CMAKE_BINARY_DIR}/libmemcached")

set(LIBMEMCACHED_SOURCE "${LIBMEMCACHED_BASE}/src")
set(LIBMEMCACHED_BINARY "${LIBMEMCACHED_BASE}/build")
set(LIBMEMCACHED_INSTALL "${LIBMEMCACHED_BASE}/install")

ExternalProject_Add(libmemcached
  URL ${LIBMEMCACHED_URL} ${LIBMEMCACHED_URL_MIRROR}
  URL_HASH SHA256=e22c0bb032fde08f53de9ffbc5a128233041d9f33b5de022c0978a2149885f82
  SOURCE_DIR ${LIBMEMCACHED_SOURCE}
  CONFIGURE_COMMAND ${LIBMEMCACHED_SOURCE}/configure --prefix=${LIBMEMCACHED_INSTALL} --enable-shared --with-pic --libdir=${LIBMEMCACHED_INSTALL}/lib/
  PATCH_COMMAND sed -i "s/opt_servers == false/opt_servers == 0/" ${LIBMEMCACHED_SOURCE}/clients/memflush.cc
  BINARY_DIR ${LIBMEMCACHED_BINARY}
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  LOG_OUTPUT_ON_FAILURE 1
  LOG_DOWNLOAD 1
  LOG_UPDATE 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1)

set(LIBMEMCACHED_INCLUDE_DIR ${LIBMEMCACHED_INSTALL}/include CACHE INTERNAL "")
set(LIBMEMCACHED_LIBRARIES ${LIBMEMCACHED_INSTALL}/lib/libmemcached.a)
set(LIBMEMCACHED_FOUND TRUE)
