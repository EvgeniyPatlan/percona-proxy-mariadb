# Tarball package configuration
message(STATUS "Generating tar.gz packages")
set(PERCONA_PROXY_BINDIR /bin CACHE PATH "" FORCE)
set(PERCONA_PROXY_LIBDIR /lib/percona-proxy CACHE PATH "" FORCE)
set(PERCONA_PROXY_SHAREDIR /share CACHE PATH "" FORCE)
set(PERCONA_PROXY_DOCDIR /share CACHE PATH "" FORCE)
set(PERCONA_PROXY_VARDIR /var CACHE PATH "" FORCE)
set(PERCONA_PROXY_CONFDIR /etc CACHE PATH "" FORCE)
set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "" FORCE)
set(CMAKE_INSTALL_DATADIR /share CACHE PATH "" FORCE)
set(DEFAULT_LIB_SUBPATH /lib/percona-proxy CACHE PATH "" FORCE)
set(DEFAULT_LIBDIR "/usr/local/percona-proxy/lib/percona-proxy" CACHE PATH "" FORCE)
set(CPACK_GENERATOR "TGZ")

# Include the var directories in the tarball
#
# On some platforms with certain CMake versions, installing empty directories
# with tarballs does not work. As a workaround, the .cmake-tgz-workaround file
# is installed into the would-be empty directories.
file(WRITE ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround "")
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/cache/percona-proxy COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/log/percona-proxy COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/run/percona-proxy COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/lib/percona-proxy COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/lib/percona-proxy/percona-proxy.cnf.d COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION etc/percona-proxy.modules.d COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION var/lib/plugin COMPONENT core)
install(FILES ${CMAKE_BINARY_DIR}/.cmake-tgz-workaround DESTINATION ${DEFAULT_CONNECTOR_PLUGIN_SUBPATH} COMPONENT core)

if(TARBALL_FILE_NAME)
  set(CPACK_PACKAGE_FILE_NAME "${TARBALL_FILE_NAME}")
elseif(DISTRIB_SUFFIX)
  set(CPACK_PACKAGE_FILE_NAME "percona-proxy-${PERCONA_PROXY_VERSION}.${DISTRIB_SUFFIX}")
else()
  set(CPACK_PACKAGE_FILE_NAME "percona-proxy-${PERCONA_PROXY_VERSION}")
endif()
