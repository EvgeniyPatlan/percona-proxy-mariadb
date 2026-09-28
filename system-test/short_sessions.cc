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
 * @file short_sessions.cpp Executes a lof of short queries, use own short session for every query (some
 * relations to bug#424)
 *
 * - using RSplit create table
 * - close connection
 * - do 100 times: open connections to RWSplit, execute short INSERT, close connection
 * - Select inserted rows through all services
 * - check if Maxscale alive
 */


#include <iostream>
#include <maxtest/testconnections.hh>
#include <maxtest/sql_t1.hh>

using namespace std;

int main(int argc, char* argv[])
{
    int iterations = 100;

    TestConnections test(argc, argv);
    test.reset_timeout();
    test.repl->connect();

    MYSQL* conn = test.percona_proxy->open_rwsplit_connection();
    execute_query(conn, "USE test;");
    create_t1(conn);
    mysql_close(conn);

    test.tprintf("Executing %d inserts", iterations);

    for (int i = 0; i < iterations; i++)
    {
        char sql[100];
        sprintf(sql, "INSERT INTO t1 (x1, fl) VALUES(%d, 1);", i);

        test.reset_timeout();
        conn = test.percona_proxy->open_rwsplit_connection();
        execute_query(conn, "%s", sql);
        mysql_close(conn);
    }

    test.reset_timeout();
    test.add_result(test.percona_proxy->connect_percona_proxy(), "Failed to connect to Percona Proxy");

    test.tprintf("Checking t1 table using RWSplit router");
    test.reset_timeout();
    test.add_result(execute_select_query_and_check(test.percona_proxy->conn_rwsplit,
                                                   (char*) "SELECT * FROM t1;",
                                                   iterations),
                    "t1 is wrong");

    test.tprintf("Checking t1 table using ReadConn router in master mode");
    test.reset_timeout();
    test.add_result(execute_select_query_and_check(test.percona_proxy->conn_master,
                                                   (char*) "SELECT * FROM t1;",
                                                   iterations),
                    "t1 is wrong");

    test.tprintf("Checking t1 table using ReadConn router in slave mode");
    test.reset_timeout();
    test.add_result(execute_select_query_and_check(test.percona_proxy->conn_slave,
                                                   (char*) "SELECT * FROM t1;",
                                                   iterations),
                    "t1 is wrong");

    test.reset_timeout();
    test.percona_proxy->close_percona_proxy_connections();
    test.check_percona_proxy_alive();

    return test.global_result;
}
