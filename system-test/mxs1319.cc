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
 * Check that SQL_MODE='PAD_CHAR_TO_FULL_LENGTH' doesn't break authentication
 */

#include <maxtest/testconnections.hh>

int main(int argc, char* argv[])
{
    TestConnections test(argc, argv);

    test.tprintf("Changing SQL_MODE to PAD_CHAR_TO_FULL_LENGTH and restarting Percona Proxy");
    test.repl->connect();
    test.repl->execute_query_all_nodes("SET GLOBAL SQL_MODE='PAD_CHAR_TO_FULL_LENGTH'");
    test.percona_proxy->restart_percona_proxy();

    test.tprintf("Connecting to Percona Proxy and executing a query");
    test.percona_proxy->connect_percona_proxy();
    test.try_query(test.percona_proxy->conn_rwsplit, "SELECT 1");
    test.percona_proxy->close_percona_proxy_connections();

    test.repl->execute_query_all_nodes("SET GLOBAL SQL_MODE=DEFAULT");
    return test.global_result;
}
