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
 * @file max621_unreadable_cnf.cpp mxs621 regression case ("Percona Proxy fails to start silently if config file is
 * not readable")
 *
 * - make percona-proxy.cnf unreadable
 * - try to restart Maxscale
 * - check log for error
 * - retore access rights to percona-proxy.cnf
 */


#include <iostream>
#include <unistd.h>
#include <maxtest/testconnections.hh>

using namespace std;

int main(int argc, char* argv[])
{
    TestConnections test(argc, argv);

    test.reset_timeout();
    test.percona_proxy->ssh_node_f(true, "chmod 400 /etc/percona-proxy.cnf");
    test.reset_timeout();
    test.percona_proxy->restart_percona_proxy();
    test.reset_timeout();
    int rv = test.percona_proxy->ssh_node_f(true,
                                       "journalctl --since '-5s' -u percona-proxy | "
                                       "grep \"Opening file '/etc/percona-proxy.cnf' for reading failed\"");
    test.expect(rv == 0,
                "\"Opening file '/etc/percona-proxy.cnf' for reading failed\" not found in stderr output.");
    test.reset_timeout();
    test.percona_proxy->ssh_node_f(true, "chmod 777 /etc/percona-proxy.cnf");

    return test.global_result;
}
