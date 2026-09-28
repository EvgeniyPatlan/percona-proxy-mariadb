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
 * @file bug676.cpp  reproducing attempt for bug676
 * - connect to RWSplit
 * - stop node0
 * - sleep 20 seconds
 * - reconnect
 * - check if 'USE test' is ok
 * - check Percona Proxy is alive
 */

#include <maxtest/testconnections.hh>
#include <maxtest/galera_cluster.hh>

int main(int argc, char* argv[])
{
    TestConnections test(argc, argv);

    test.reset_timeout();

    test.percona_proxy->connect_percona_proxy();
    test.tprintf("Stopping node 0");
    test.galera->block_node(0);
    test.percona_proxy->close_percona_proxy_connections();

    test.tprintf("Waiting until the monitor picks a new master");
    test.percona_proxy->wait_for_monitor();

    test.reset_timeout();

    test.percona_proxy->connect_percona_proxy();
    test.try_query(test.percona_proxy->conn_rwsplit, "USE test");
    test.try_query(test.percona_proxy->conn_rwsplit, "show processlist;");
    test.percona_proxy->close_percona_proxy_connections();

    test.galera->unblock_node(0);

    return test.global_result;
}
