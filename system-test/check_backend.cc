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
 * @file check_backend.cpp simply checks if backend is alive
 */


#include <iostream>
#include <maxtest/testconnections.hh>

int main(int argc, char *argv[])
{

    TestConnections * Test = new TestConnections(argc, argv);

    // Reset server settings by replacing the config files
    Test->repl->reset_all_servers_settings();

    Test->tprintf("Connecting to Percona Proxy percona_proxies->routers[0] with Master/Slave backend\n");
    Test->percona_proxy->connect_percona_proxy();
    Test->tprintf("Testing connections\n");

    Test->add_result(Test->test_percona_proxy_connections(true, true, true), "Can't connect to backend\n");

    Test->tprintf("Connecting to Percona Proxy router with Galera backend\n");
    MYSQL * g_conn = open_conn(4016, Test->percona_proxy->ip4(), Test->percona_proxy->user_name(),
                               Test->percona_proxy->password(), Test->percona_proxy_ssl);
    if (g_conn != NULL )
    {
        Test->tprintf("Testing connection\n");
        Test->add_result(Test->try_query(g_conn, (char *) "SELECT 1"),
                         (char *) "Error executing query against RWSplit Galera\n");
    }

    Test->tprintf("Closing connections\n");
    Test->percona_proxy->close_percona_proxy_connections();
    Test->check_percona_proxy_alive();

    auto ver = Test->percona_proxy->ssh_output("percona-proxy --version-full", false);
    Test->tprintf("Percona_proxy_full_version_start:\n%s\nPercona_proxy_full_version_end\n", ver.output.c_str());

    int rval = Test->global_result;
    delete Test;
    return rval;
}
