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
 * MXS-2417: Ignore persisted configs with load_persisted_configs=false
 * https://jira.mariadb.org/browse/MXS-2417
 */

#include <maxtest/testconnections.hh>

int main(int argc, char* argv[])
{
    TestConnections test(argc, argv);

    test.tprintf("Creating a server and verifying it exists");
    test.check_percona_proxyctl("create server server1234 127.0.0.1 3306");
    test.check_percona_proxyctl("show server server1234");

    test.tprintf("Restarting Percona Proxy");
    test.percona_proxy->restart_percona_proxy();

    test.tprintf("Creating the server again and verifying it is successful");
    test.check_percona_proxyctl("create server server1234 127.0.0.1 3306");
    test.check_percona_proxyctl("show server server1234");

    return test.global_result;
}
