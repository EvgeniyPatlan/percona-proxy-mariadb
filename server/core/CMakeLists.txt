configure_file(internal/defaults.hh.in internal/defaults.hh @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(percona-proxy-common SHARED
  admin.cc
  adminusers.cc
  backend.cc
  base_user_manager.cc
  buffer.cc
  cachingparser.cc
  config.cc
  config2.cc
  config_runtime.cc
  configmanager.cc
  dcb.cc
  event.cc
  filter.cc
  hint.cc
  history.cc
  httprequest.cc
  httpresponse.cc
  http_sql.cc
  indexedstorage.cc
  json.cc
  json_api.cc
  jwt.cc
  key_manager.cc
  listener.cc
  load_utils.cc
  log.cc
  mainworker.cc
  percona_proxy_pcre2.cc
  misc.cc
  modulecmd.cc
  monitor.cc
  monitormanager.cc
  parser.cc
  profiler.cc
  queryclassifier.cc
  resource.cc
  response_stat.cc
  router.cc
  routingworker.cc
  secrets.cc
  server.cc
  servermanager.cc
  service.cc
  session.cc
  session_stats.cc
  sql_conn_manager.cc
  sql_etl.cc
  sql_etl_generic.cc
  ssl.cc
  target.cc
  test.cc
  testparser.cc
  threadpool.cc
  users.cc
  utils.cc
  websocket.cc
  response_distribution.cc
  )

target_link_libraries(percona-proxy-common
  maxbase
  maxsql
  maxsimd
  mariadbprotocol
  mariadbauth
  ${MARIADB_CONNECTOR_LIBRARIES}
  ${LZMA_LINK_FLAGS}
  ${PCRE2_LIBRARIES}
  ${JANSSON_LIBRARIES}
  ssl
  pthread
  crypt
  crypto
  z
  rt
  m
  sqlite3
  stdc++
  stdc++fs
  ${MICROHTTPD_LIBRARIES}
  ${GNUTLS_LIBRARIES}
  ${LIBATOMIC_LIBRARIES}
  uuid
)

set_target_properties(percona-proxy-common PROPERTIES LINK_FLAGS -Wl,--exclude-libs=libmariadbclient.a)

if (BUILD_KMIP_KEY_MANAGER AND BUNDLE)
  include(${CMAKE_SOURCE_DIR}/cmake/BuildLibKMIP.cmake)
  add_dependencies(percona-proxy-common libkmip)
  add_compile_definitions(BUILD_KMIP_KEY_MANAGER)
  target_link_libraries(percona-proxy-common ${LIBKMIP_LIBRARIES})
  target_sources(percona-proxy-common PRIVATE key_manager_kmip.cc)
  target_include_directories(percona-proxy-common PUBLIC ${LIBKMIP_INCLUDE_DIR})
endif()

if (BUILD_VAULT_KEY_MANAGER AND BUNDLE)
  include(${CMAKE_SOURCE_DIR}/cmake/BuildLibVault.cmake)
  if (LIBVAULT_FOUND)
    add_dependencies(percona-proxy-common libvault)
    add_compile_definitions(BUILD_VAULT_KEY_MANAGER)
    target_link_libraries(percona-proxy-common ${LIBVAULT_LIBRARIES})
    target_sources(percona-proxy-common PRIVATE key_manager_vault.cc)
    target_include_directories(percona-proxy-common PUBLIC ${LIBVAULT_INCLUDE_DIR})

    # The libvault header uses pragmas unknown to GCC which cause warnings and
    # due to -Werror, these cause the build to fail.
    set_source_files_properties(key_manager_vault.cc PROPERTIES COMPILE_FLAGS -Wno-unknown-pragmas)
  else()
    message(STATUS "Will not build libvault, compiler is too old")
  endif()
endif()

find_library(HAVE_LIBDL NAMES dl)
if (HAVE_LIBDL)
  # libdl just exposes libc functionality on most systems.  This means that if
  # we can't find it, it's likely that libc already exposes the symbols.
  target_link_libraries(percona-proxy-common dl)
endif()

# Using initial-exec instead of the default global-dynamic tls-model
# reduces the cost of using thread-local variables in dynamic libraries.
target_compile_options(percona-proxy-common PRIVATE "-ftls-model=initial-exec")

add_dependencies(percona-proxy-common connector-c maxbase maxsimd)
set_target_properties(percona-proxy-common PROPERTIES VERSION "1.0.0")
install_module(percona-proxy-common core)

if (FORCE_BUNDLE)
  add_dependencies(percona-proxy-common libmicrohttpd)
endif()

add_executable(percona-proxy gateway.cc)

if(WITH_JEMALLOC)
  target_link_libraries(percona-proxy ${JEMALLOC_LIBRARIES})
elseif(WITH_TCMALLOC)
  target_link_libraries(percona-proxy ${TCMALLOC_LIBRARIES})
endif()

target_link_libraries(percona-proxy percona-proxy-common)
install_executable(percona-proxy core)

add_executable(percona-proxy-keys percona-proxy-keys.cc)
target_link_libraries(percona-proxy-keys percona-proxy-common)
install_executable(percona-proxy-keys core)

add_executable(percona-proxy-passwd percona-proxy-passwd.cc)
target_link_libraries(percona-proxy-passwd percona-proxy-common)
install_executable(percona-proxy-passwd core)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
