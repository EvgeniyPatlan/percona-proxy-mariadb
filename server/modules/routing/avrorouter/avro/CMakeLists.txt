include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(maxavro maxavro.cc maxavro_schema.cc maxavro_record.cc maxavro_file.cc)
target_link_libraries(maxavro percona-proxy-common ${JANSSON_LIBRARIES})

add_executable(percona-proxy-avrocheck percona-proxy-avrocheck.cc)
target_link_libraries(percona-proxy-avrocheck maxavro percona-proxy-common)
install_executable(percona-proxy-avrocheck core)
add_subdirectory(test)
