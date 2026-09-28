add_executable(test_cycle_find test_cycle_find.cc)
target_link_libraries(test_cycle_find mariadbmon percona-proxy-common)
add_test(test_mariadbmon_cycle_find test_cycle_find)

add_executable(test_gtid test_gtid.cc)
target_link_libraries(test_gtid mariadbmon percona-proxy-common)
add_test(test_mariadbmon_gtid test_gtid)
