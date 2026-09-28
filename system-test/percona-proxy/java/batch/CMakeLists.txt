add_java_test(batchinsert BatchInsert.java percona-proxy.java.batch.BatchInsert batchinsert)
set_tests_properties(batchinsert PROPERTIES TIMEOUT 300)
