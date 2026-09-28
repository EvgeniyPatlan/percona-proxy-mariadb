add_executable(profile_trxboundaryparser profile_trxboundaryparser.cc)
target_link_libraries(profile_trxboundaryparser percona-proxy-common)

add_executable(test_parse_kill test_parse_kill.cc)
add_executable(test_setparser test_setparser.cc)
add_executable(test_setsqlmodeparser test_setsqlmodeparser.cc)
add_executable(test_sqlmodeparser test_sqlmodeparser.cc)

target_link_libraries(test_parse_kill percona-proxy-common)
target_link_libraries(test_setsqlmodeparser percona-proxy-common)
target_link_libraries(test_setparser percona-proxy-common)
target_link_libraries(test_sqlmodeparser percona-proxy-common)

add_test(test_parse_kill test_parse_kill)
add_test(test_setparser test_setparser)
add_test(test_setsqlmodeparser test_setsqlmodeparser)
add_test(test_sqlmodeparser test_sqlmodeparser)

add_executable(test_sqlite_strlike test_sqlite_strlike.cc ../sqlite_strlike.cc)
target_link_libraries(test_sqlite_strlike percona-proxy-common)
add_test(test_sqlite_strlike test_sqlite_strlike)

add_executable(test_host_pattern test_host_pattern.cc)
target_link_libraries(test_host_pattern percona-proxy-common)
add_test(test_host_pattern test_host_pattern)

add_executable(test_trxtracking test_trxtracking.cc)
target_link_libraries(test_trxtracking percona-proxy-common)
add_test(test_trxtracking test_trxtracking)

set(QC_TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../parser_plugin/test)
add_executable(test_trxcompare test_trxcompare.cc ${QC_TEST_PATH}/testreader.cc)
target_link_libraries(test_trxcompare percona-proxy-common)
add_test(test_trxcompare_create test_trxcompare ${QC_TEST_PATH}/create.test)
add_test(test_trxcompare_delete test_trxcompare ${QC_TEST_PATH}/delete.test)
add_test(test_trxcompare_insert test_trxcompare ${QC_TEST_PATH}/insert.test)
add_test(test_trxcompare_join test_trxcompare ${QC_TEST_PATH}/join.test)
add_test(test_trxcompare_select test_trxcompare ${QC_TEST_PATH}/select.test)
add_test(test_trxcompare_set test_trxcompare ${QC_TEST_PATH}/set.test)
add_test(test_trxcompare_update test_trxcompare ${QC_TEST_PATH}/update.test)
add_test(test_trxcompare_percona_proxy test_trxcompare ${QC_TEST_PATH}/percona-proxy.test)

add_subdirectory(canonical_tests)
