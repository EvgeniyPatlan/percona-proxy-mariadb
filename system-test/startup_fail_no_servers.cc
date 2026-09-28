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
 * @file bug662.cpp regression case for bug 662 ("Percona Proxy hangs in startup if backend server is not
 * responsive"), covers also bug680 ("RWSplit can't load DB user if backend is not available at Percona Proxy
 * start")
 *
 * - Block all Mariadb servers
 * - Restart Percona Proxy
 * - Unblock Mariadb servers
 * - Sleep and check if Maxscale is alive
 */

#include <maxtest/testconnections.hh>

int main(int argc, char* argv[])
{
    TestConnections* Test = new TestConnections(argc, argv);
    Test->percona_proxy->connect_percona_proxy();

    for (int i = 0; i < Test->repl->N; i++)
    {
        Test->reset_timeout();
        Test->tprintf("Setup firewall to block mysql on node %d\n", i);
        Test->repl->block_node(i);
    }

    Test->reset_timeout();
    Test->tprintf("Restarting Percona Proxy");
    Test->percona_proxy->restart_percona_proxy();

    Test->tprintf("Checking if Percona Proxy is alive by connecting to with percona-proxyctl\n");
    Test->check_percona_proxyctl("show servers");

    for (int i = 0; i < Test->repl->N; i++)
    {
        Test->reset_timeout();
        Test->tprintf("Setup firewall back to allow mysql on node %d\n", i);
        Test->repl->unblock_node(i);
    }

    sleep(3);

    Test->reset_timeout();
    Test->check_percona_proxy_alive();

    int rval = Test->global_result;
    delete Test;
    return rval;
}
