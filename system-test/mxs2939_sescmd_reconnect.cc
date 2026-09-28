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
 * MXS-2939: Test that session commands trigger a reconnection
 */

#include <maxtest/testconnections.hh>

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);

    test.percona_proxy->connect_rwsplit();

    // Make sure we have at least one fully opened connection
    test.try_query(test.percona_proxy->conn_rwsplit, "select 1");

    // Block and unblock all nodes to sever all connections
    for (int i = 0; i < test.repl->N; i++)
    {
        test.repl->block_node(i);
    }

    test.percona_proxy->wait_for_monitor();

    test.repl->unblock_all_nodes();
    test.percona_proxy->wait_for_monitor();

    // Make sure that session commands trigger a reconnection if there are no open connections
    test.reset_timeout();
    test.try_query(test.percona_proxy->conn_rwsplit, "set @a = 1");
    test.percona_proxy->disconnect();

    return test.global_result;
}
