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
 * MXS-1476: priority value ignored when a Galera node rejoins with a lower wsrep_local_index than current
 * master
 *
 * https://jira.mariadb.org/browse/MXS-1476
 */

#include <maxtest/testconnections.hh>
#include <maxtest/galera_cluster.hh>

void list_servers(TestConnections& test)
{
    auto output = test.percona_proxy->ssh_output("percona-proxyctl list servers");
    test.tprintf("%s", output.output.c_str());
}

void do_test(TestConnections& test, int master, int slave)
{
    test.percona_proxy->connect_percona_proxy();
    test.try_query(test.percona_proxy->conn_rwsplit, "DROP TABLE IF EXISTS test.t1");
    test.try_query(test.percona_proxy->conn_rwsplit, "CREATE TABLE test.t1 (id int)");
    test.try_query(test.percona_proxy->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");

    test.tprintf("Stop a slave node and perform an insert");
    test.galera->block_node(slave);
    test.percona_proxy->wait_for_monitor();
    list_servers(test);

    test.try_query(test.percona_proxy->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");

    test.tprintf("Start the slave node and perform another insert");
    test.galera->unblock_node(slave);
    test.percona_proxy->wait_for_monitor();
    list_servers(test);

    test.try_query(test.percona_proxy->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test.percona_proxy->close_percona_proxy_connections();

    test.tprintf("Stop the master node and perform an insert");
    test.galera->block_node(master);
    test.percona_proxy->wait_for_monitor();
    list_servers(test);

    test.percona_proxy->connect_percona_proxy();
    test.try_query(test.percona_proxy->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");

    test.tprintf("Start the master node and perform another insert (expecting failure)");
    test.galera->unblock_node(master);
    test.percona_proxy->wait_for_monitor();
    list_servers(test);

    test.add_result(execute_query_silent(test.percona_proxy->conn_rwsplit,
                                         "INSERT INTO test.t1 VALUES (1)") == 0,
                    "Query should fail");
    test.percona_proxy->close_percona_proxy_connections();

    test.percona_proxy->connect_percona_proxy();
    test.try_query(test.percona_proxy->conn_rwsplit, "DROP TABLE test.t1");
}

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    test.galera->stop_node(2);
    test.galera->stop_node(3);

    do_test(test, 1, 0);

    test.tprintf("Swap the priorities around and run the test again");
    test.percona_proxy->ssh_node_f(true,
                              "sed -i 's/priority=1/priority=3/' /etc/percona-proxy.cnf;"
                              "sed -i 's/priority=2/priority=1/' /etc/percona-proxy.cnf;"
                              "sed -i 's/priority=3/priority=2/' /etc/percona-proxy.cnf;");
    test.percona_proxy->restart_percona_proxy();

    // Give the Galera nodes some time to stabilize
    sleep(5);

    do_test(test, 0, 1);

    test.galera->start_node(2);
    test.galera->start_node(3);
    return test.global_result;
}
