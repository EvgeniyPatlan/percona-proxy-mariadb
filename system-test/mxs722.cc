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
 * @file mxs722.cpp Percona Proxy configuration check functionality test
 *
 * - Get baseline for test from a valid config
 * - Test wrong parameter name
 * - Test wrong router_options value
 * - Test wrong filter parameter
 * - Test missing config file
 */


#include <iostream>
#include <unistd.h>
#include <maxtest/testconnections.hh>

using namespace std;

int main(int argc, char* argv[])
{
    TestConnections* test = new TestConnections(argc, argv);
    test->percona_proxy->stop();

    /** Copy original config so we can easily reset the testing environment */
    test->percona_proxy->ssh_node_f(true, "cp /etc/percona-proxy.cnf /tmp/percona-proxy.cnf");
    test->percona_proxy->ssh_node_f(true, "chmod a+rw /tmp/percona-proxy.cnf");

    const char* percona_proxy_cmd =
        "ASAN_OPTIONS=detect_leaks=0 percona-proxy -c --user=percona-proxy --piddir=/tmp -f /tmp/percona-proxy.cnf";

    /** Get a baseline result with a good configuration */
    int baseline = test->percona_proxy->ssh_node_f(true, "%s", percona_proxy_cmd);

    /** Configure bad parameter for a listener */
    test->percona_proxy->ssh_node_f(true, "sed -i -e 's/service/ecivres/' /tmp/percona-proxy.cnf");
    test->add_result(
        baseline == test->percona_proxy->ssh_node_f(true, "%s", percona_proxy_cmd),
        "Bad parameter name should be detected.\n");
    test->percona_proxy->ssh_node_f(true, "cp /etc/percona-proxy.cnf /tmp/percona-proxy.cnf");

    /** Set router_options to a bad value */
    test->percona_proxy->ssh_node_f(true,
                               "sed -i -e 's/router_options.*/router_options=bad_option=true/' /tmp/percona-proxy.cnf");
    test->add_result(
        baseline == test->percona_proxy->ssh_node_f(true, "%s", percona_proxy_cmd),
        "Bad router_options should be detected.\n");

    test->percona_proxy->ssh_node_f(true, "cp /etc/percona-proxy.cnf /tmp/percona-proxy.cnf");

    /** Configure bad filter parameter */
    test->percona_proxy->ssh_node_f(true, "sed -i -e 's/filebase/basefile/' /tmp/percona-proxy.cnf");
    test->add_result(
        baseline == test->percona_proxy->ssh_node_f(true, "%s", percona_proxy_cmd),
        "Bad filter parameter should be detected.\n");

    /** Remove configuration file */
    test->percona_proxy->ssh_node_f(true, "rm -f /tmp/percona-proxy.cnf");
    test->add_result(
        baseline == test->percona_proxy->ssh_node_f(true, "%s", percona_proxy_cmd),
        "Missing configuration file should be detected.\n");

    int rval = test->global_result;
    delete test;
    return rval;
}
