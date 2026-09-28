/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 * Copyright (c) 2023 MariaDB plc, Finnish Branch
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2026-09-21
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 * @file rwsplit_readonly.cpp Test of the read-only mode for readwritesplit when master fails
 * - check INSERTs via RWSplit
 * - block Master
 * - check SELECT and INSERT with -- fail_instantly, -- error_on_write, -- fail_on_write
 */


#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <maxtest/testconnections.hh>

void test_all_ok(TestConnections* Test)
{
    /** Insert should work */
    auto rc_master = Test->percona_proxy->conn_master;
    auto rc_slave = Test->percona_proxy->conn_slave;

    Test->tprintf("Testing that writes and reads to all services work\n");
    Test->add_result(execute_query_silent(Test->percona_proxy->conn_rwsplit,
                                          "INSERT INTO test.readonly VALUES (1) -- fail_instantly"),
                     "Query to service with 'fail_instantly' should succeed\n");
    Test->add_result(execute_query_silent(rc_master,
                                          "INSERT INTO test.readonly VALUES (1) -- fail_on_write"),
                     "Query to service with 'fail_on_write' should succeed\n");
    Test->add_result(execute_query_silent(rc_slave,
                                          "INSERT INTO test.readonly VALUES (1) -- error_on_write"),
                     "Query to service with 'error_on_write' should succeed\n");
    Test->add_result(execute_query_silent(Test->percona_proxy->conn_rwsplit,
                                          "SELECT * FROM test.readonly -- fail_instantly"),
                     "Query to service with 'fail_instantly' should succeed\n");
    Test->add_result(execute_query_silent(rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "Query to service with 'fail_on_write' should succeed\n");
    Test->add_result(execute_query_silent(rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "Query to service with 'error_on_write' should succeed\n");
}

void test_basic(TestConnections* Test)
{
    /** Check that everything is OK before blocking the master */
    Test->percona_proxy->connect_percona_proxy();
    test_all_ok(Test);

    /** Block master */
    Test->repl->block_node(0);
    Test->percona_proxy->wait_for_monitor();

    MYSQL*& conn_rc_master = Test->percona_proxy->conn_master;
    MYSQL*& conn_rc_slave = Test->percona_proxy->conn_slave;

    /** Select to service with 'fail_instantly' should close the connection */
    Test->tprintf("SELECT to 'fail_instantly'\n");
    Test->add_result(!execute_query_silent(Test->percona_proxy->conn_rwsplit,
                                           "SELECT * FROM test.readonly -- fail_instantly"),
                     "SELECT to service with 'fail_instantly' should fail\n");

    /** Other services should still work */
    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    /** Insert to 'fail_on_write' should fail and close the connection */
    Test->tprintf("INSERT to 'fail_on_write'\n");
    Test->add_result(!execute_query_silent(conn_rc_master,
                                           "INSERT INTO test.readonly VALUES (1) -- fail_on_write"),
                     "INSERT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(!execute_query_silent(conn_rc_master,
                                           "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should fail after an INSERT\n");

    /** Insert to 'error_on_write' should fail but subsequent SELECTs should work */
    Test->tprintf("INSERT to 'error_on_write'\n");
    Test->add_result(!execute_query_silent(conn_rc_slave,
                                           "INSERT INTO test.readonly VALUES (1) -- error_on_write"),
                     "INSERT to service with 'error_on_write' should fail\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed after an INSERT\n");

    /** Close connections and try to create new ones */
    Test->percona_proxy->close_percona_proxy_connections();
    Test->tprintf("Opening connections while master is blocked\n");
    Test->add_result(Test->percona_proxy->connect_rwsplit() == 0,
                     "Connection to 'fail_instantly' service should fail\n");
    Test->add_result(Test->percona_proxy->connect_readconn_master() != 0,
                     "Connection to 'fail_on_write' service should succeed\n");
    Test->add_result(Test->percona_proxy->connect_readconn_slave() != 0,
                     "Connection to 'error_on_write' service should succeed\n");


    /** The {fail|error}_on_write services should work and allow reads */
    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    Test->percona_proxy->close_percona_proxy_connections();
    Test->repl->unblock_node(0);
    Test->percona_proxy->wait_for_monitor();

    /** Check that everything is OK after unblocking */
    Test->percona_proxy->connect_percona_proxy();
    test_all_ok(Test);
    Test->percona_proxy->close_percona_proxy_connections();
}

void test_complex(TestConnections* Test)
{
    /** Check that everything works before test */
    Test->percona_proxy->connect_percona_proxy();
    test_all_ok(Test);

    /** Block master */
    Test->repl->block_node(0);
    Test->percona_proxy->wait_for_monitor();

    MYSQL*& conn_rc_master = Test->percona_proxy->conn_master;
    MYSQL*& conn_rc_slave = Test->percona_proxy->conn_slave;

    /** Select to service with 'fail_instantly' should close the connection */
    Test->tprintf("SELECT to 'fail_instantly'\n");
    Test->add_result(!execute_query_silent(Test->percona_proxy->conn_rwsplit,
                                           "SELECT * FROM test.readonly -- fail_instantly"),
                     "SELECT to service with 'fail_instantly' should fail\n");

    /** The {fail|error}_on_write services should allow reads */
    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    /** Unblock node and try to read */
    Test->repl->unblock_node(0);
    Test->percona_proxy->wait_for_monitor();

    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    /** Block slaves */
    Test->percona_proxy->close_percona_proxy_connections();
    Test->repl->block_node(1);
    Test->repl->block_node(2);
    Test->repl->block_node(3);
    Test->percona_proxy->wait_for_monitor();

    /** Reconnect to Percona Proxy */
    Test->percona_proxy->connect_percona_proxy();

    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    Test->repl->unblock_node(1);
    Test->repl->unblock_node(2);
    Test->repl->unblock_node(3);
    Test->percona_proxy->wait_for_monitor();


    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_master,
                                          "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should succeed\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(execute_query_silent(conn_rc_slave,
                                          "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should succeed\n");

    /** Block all nodes */
    Test->repl->block_node(0);
    Test->repl->block_node(1);
    Test->repl->block_node(2);
    Test->repl->block_node(3);
    Test->percona_proxy->wait_for_monitor();

    /** SELECTs should fail*/
    Test->tprintf("SELECT to 'fail_on_write'\n");
    Test->add_result(!execute_query_silent(conn_rc_master,
                                           "SELECT * FROM test.readonly -- fail_on_write"),
                     "SELECT to service with 'fail_on_write' should fail\n");
    Test->tprintf("SELECT to 'error_on_write'\n");
    Test->add_result(!execute_query_silent(conn_rc_slave,
                                           "SELECT * FROM test.readonly -- error_on_write"),
                     "SELECT to service with 'error_on_write' should fail\n");
    Test->repl->unblock_node(0);
    Test->repl->unblock_node(1);
    Test->repl->unblock_node(2);
    Test->repl->unblock_node(3);
    Test->percona_proxy->wait_for_monitor();

    /** Reconnect and check that everything works after the test */
    Test->percona_proxy->close_percona_proxy_connections();
    Test->percona_proxy->connect_percona_proxy();
    test_all_ok(Test);
    Test->percona_proxy->close_percona_proxy_connections();
}

int main(int argc, char* argv[])
{

    TestConnections* Test = new TestConnections(argc, argv);

    /** Prepare for tests */
    Test->percona_proxy->connect_percona_proxy();
    execute_query_silent(Test->percona_proxy->conn_rwsplit, "DROP TABLE IF EXISTS test.readonly");
    execute_query_silent(Test->percona_proxy->conn_rwsplit, "CREATE TABLE test.readonly(id int)");
    Test->percona_proxy->close_percona_proxy_connections();

    Test->repl->connect();
    Test->repl->sync_slaves();
    Test->repl->disconnect();

    /** Basic tests */
    test_basic(Test);

    /** More complex tests */
    test_complex(Test);

    Test->repl->connect();
    execute_query(Test->repl->nodes[0], "DROP TABLE test.readonly");
    Test->repl->disconnect();

    int rval = Test->global_result;
    delete Test;
    return rval;
}
