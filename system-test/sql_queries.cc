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
 * @file sql_queries.cpp  Execute long sql queries as well as "use" command (also used for bug648 "use
 * database is sent forever with tee filter to a readwrite split service")
 * - also used for 'sql_queries_pers1' and 'sql_queries_pers10' tests (with 'persistpoolmax=1' and
 *'persistpoolmax=10' for all servers)
 * - for bug648:
 * @verbatim
 *  [RW Split Router]
 *  type=service
 *  router= readwritesplit
 *  servers=server1,     server2,              server3,server4
 *  user=skysql
 *  passwd=skysql
 *  filters=TEE
 *
 *  [TEE]
 *  type=filter
 *  module=tee
 *  service=RW Split Router
 *  @endverbatim
 *
 * - create t1 table and INSERT a lot of date into it
 * @verbatim
 *  INSERT INTO t1 (x1, fl) VALUES (0, 0), (1, 0), ...(15, 0);
 *  INSERT INTO t1 (x1, fl) VALUES (0, 1), (1, 1), ...(255, 1);
 *  INSERT INTO t1 (x1, fl) VALUES (0, 2), (1, 2), ...(4095, 2);
 *  INSERT INTO t1 (x1, fl) VALUES (0, 3), (1, 3), ...(65535, 3);
 *  @endverbatim
 * - check date in t1 using all Percona Proxy services and direct connections to backend nodes
 * - using RWSplit connections:
 *   + DROP TABLE t1
 *   + DROP DATABASE IF EXISTS test1;
 *   + CREATE DATABASE test1;
 * - execute USE test1 for all Percona Proxy service and backend nodes
 * - create t1 table and INSERT a lot of date into it
 * - check that 't1' exists in 'test1' DB and does not exist in 'test'
 * - executes queries with syntax error against all Percona Proxy services
 *   + "DROP DATABASE I EXISTS test1;"
 *   + "CREATE TABLE "
 * - check if Percona Proxy is alive
 */

#include <iostream>
#include <maxtest/testconnections.hh>
#include <maxtest/sql_t1.hh>

using namespace std;


int main(int argc, char* argv[])
{
    TestConnections* Test = new TestConnections(argc, argv);
    int i, j;
    int N = 4;
    int iterations = 4;

    if (Test->smoke)
    {
        iterations = 1;
        N = 2;
    }

    Test->tprintf("Starting test\n");
    for (i = 0; i < iterations; i++)
    {

        Test->reset_timeout();
        Test->tprintf("Connection to backend\n");
        Test->repl->connect();
        Test->tprintf("Connection to Percona Proxy\n");
        if (Test->percona_proxy->connect_percona_proxy() != 0)
        {
            Test->add_result(1, "Error connecting to Percona Proxy");
            break;
        }

        Test->tprintf("Filling t1 with data\n");
        Test->add_result(Test->insert_select(N), "insert-select check failed\n");

        Test->tprintf("Creating database test1\n");
        Test->try_query(Test->percona_proxy->conn_rwsplit, "DROP TABLE t1");
        Test->try_query(Test->percona_proxy->conn_rwsplit, "DROP DATABASE IF EXISTS test1;");
        Test->try_query(Test->percona_proxy->conn_rwsplit, "CREATE DATABASE test1;");
        Test->reset_timeout();
        Test->repl->sync_slaves();

        Test->reset_timeout();
        Test->tprintf("Testing with database 'test1'\n");
        Test->add_result(Test->use_db((char*)"test1"), "use_db failed\n");
        Test->add_result(Test->insert_select(N), "insert-select check failed\n");

        Test->add_result(Test->check_t1_table(false, (char*)"test"), "t1 is found in 'test'\n");
        Test->add_result(Test->check_t1_table(true, (char*)"test1"), "t1 is not found in 'test1'\n");

        Test->tprintf("Trying queries with syntax errors\n");
        for (j = 0; j < 3; j++)
        {
            execute_query(Test->percona_proxy->routers[j], "DROP DATABASE I EXISTS test1;");
            execute_query(Test->percona_proxy->routers[j], "CREATE TABLE ");
        }

        // close connections
        Test->percona_proxy->close_percona_proxy_connections();
        Test->repl->close_connections();
    }

    Test->log_excludes("Length (0) is 0");
    Test->log_excludes("Unable to parse query");
    Test->log_excludes("query string allocation failed");

    Test->check_percona_proxy_alive();

    Test->percona_proxy->restart_percona_proxy();
    Test->check_percona_proxy_alive();

    int rval = Test->global_result;
    delete Test;
    return rval;
}
