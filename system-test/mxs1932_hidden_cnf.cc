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
 * MXS-1932: Hidden files are not ignored
 *
 * https://jira.mariadb.org/browse/MXS-1932
 */

#include <maxtest/testconnections.hh>

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    TestConnections::skip_percona_proxy_start(true);
    TestConnections test(argc, argv);

    // Create a file with a guaranteed bad configuration (turbochargers are not yet supported)
    ofstream cnf("hidden.cnf");
    cnf << "[something]" << endl;
    cnf << "type=turbocharger" << endl;
    cnf << "target=percona-proxy" << endl;
    cnf << "speed=maximum" << endl;
    cnf.close();

    // Copy the configuration to Percona Proxy
    test.percona_proxy->copy_to_node("hidden.cnf", test.percona_proxy->access_homedir());

    // Move it into the percona-proxy.cnf.d directory and make it a hidden file
    test.percona_proxy->ssh_node_f(true,
                              "mkdir -p /etc/percona-proxy.cnf.d/;"
                              "mv %s/hidden.cnf /etc/percona-proxy.cnf.d/.hidden.cnf;"
                              "chown -R percona-proxy:percona-proxy /etc/percona-proxy.cnf.d/",
                              test.percona_proxy->access_homedir());

    // Make sure the hidden configuration is not read and that Percona Proxy starts up
    test.expect(test.percona_proxy->restart_percona_proxy() == 0, "Starting Percona Proxy should succeed");

    test.percona_proxy->ssh_node_f(true, "rm -r /etc/percona-proxy.cnf.d/");
    remove("hidden.cnf");

    return test.global_result;
}
