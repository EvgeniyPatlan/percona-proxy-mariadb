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
 * MXS-1123: connect_timeout setting causes frequent disconnects
 */

#include <maxtest/testconnections.hh>

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    test.percona_proxy->connect_percona_proxy();

    test.tprintf("Waiting one second between queries, all queries should succeed");

    sleep(1);
    test.try_query(test.percona_proxy->conn_rwsplit, "select 1");
    sleep(1);
    test.try_query(test.percona_proxy->conn_master, "select 1");
    sleep(1);
    test.try_query(test.percona_proxy->conn_slave, "select 1");

    test.percona_proxy->close_percona_proxy_connections();
    return test.global_result;
}
