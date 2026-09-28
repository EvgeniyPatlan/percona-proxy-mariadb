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
 * @file sysbanch_example.cpp Run 'sysbench'
 *
 * - start sysbanch test
 * - repeat for all services
 * - DROP sysbanch tables
 * - check if Maxscale is alive
 */


#include <maxtest/testconnections.hh>
#include "sysbench_commands.h"

int main(int argc, char* argv[])
{
    TestConnections* Test = new TestConnections(argc, argv);

    char sys1[4096];

    Test->percona_proxy->ssh_node("percona-proxy --version-full", false);
    fflush(stdout);
    auto mxs_ip = Test->percona_proxy->ip4();
    Test->tprintf("Connecting to RWSplit %s\n", mxs_ip);

    sprintf(&sys1[0], SYSBENCH_PREPARE_SHORT, mxs_ip);

    Test->tprintf("Preparing sysbench tables\n%s\n", sys1);
    Test->reset_timeout();
    Test->add_result(system(sys1), "Error executing sysbench prepare\n");

    sprintf(&sys1[0], SYSBENCH_COMMAND_SHORT, mxs_ip, Test->percona_proxy->rwsplit_port);
    Test->set_log_copy_interval(300);
    Test->tprintf("Executing sysbench \n%s\n", sys1);
    if (system(sys1) != 0)
    {
        Test->tprintf("Error executing sysbench test\n");
    }

    Test->percona_proxy->connect_percona_proxy();

    printf("Dropping sysbanch tables!\n");
    fflush(stdout);

    /*
     *  Test->try_query(Test->percona_proxies->conn_rwsplit, (char *) "DROP TABLE sbtest1");
     *  if (!Test->smoke)
     *  {
     *   Test->try_query(Test->percona_proxies->conn_rwsplit, (char *) "DROP TABLE sbtest2");
     *   Test->try_query(Test->percona_proxies->conn_rwsplit, (char *) "DROP TABLE sbtest3");
     *   Test->try_query(Test->percona_proxies->conn_rwsplit, (char *) "DROP TABLE sbtest4");
     *  }
     */

    Test->global_result += execute_query(Test->percona_proxy->conn_rwsplit, (char*) "DROP TABLE sbtest1");

    printf("closing connections to Percona Proxy!\n");
    fflush(stdout);

    Test->percona_proxy->close_percona_proxy_connections();

    Test->tprintf("Checking if Percona Proxy is still alive!\n");
    fflush(stdout);
    Test->check_percona_proxy_alive();

    int rval = Test->global_result;
    delete Test;
    return rval;
}
