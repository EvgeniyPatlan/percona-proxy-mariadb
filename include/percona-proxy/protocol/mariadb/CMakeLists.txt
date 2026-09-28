set(MARIADB_HDR_PATH ${CMAKE_INSTALL_INCLUDEDIR}/percona-proxy/protocol/mariadb)

# Install static C++ headers
file(GLOB HEADERS "*.hh" "*.h")
foreach(var ${HEADERS})
  get_filename_component(header ${var} NAME)
  install_custom_file(${header} ${MARIADB_HDR_PATH} devel)
endforeach()
