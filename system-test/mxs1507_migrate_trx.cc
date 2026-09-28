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
 * MXS-1507: Test migration of transactions
 *
 * https://jira.mariadb.org/browse/MXS-1507
 */
#include <maxtest/testconnections.hh>
#include <functional>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    string master = "server1";
    string slave = "server2";

    auto switchover = [&]() {
            test.percona_proxy->wait_for_monitor();
            int rc = test.percona_proxy->ssh_node_f(true,
                                               "percona-proxyctl call command mariadbmon switchover MySQL-Monitor %s %s",
                                               slave.c_str(),
                                               master.c_str());
            test.expect(rc == 0, "Switchover should work");
            master.swap(slave);
            test.percona_proxy->wait_for_monitor();
        };

    auto query = [&](string q) {
            return execute_query_silent(test.percona_proxy->conn_rwsplit, q.c_str()) == 0;
        };

    auto ok = [&](string q) {
            test.expect(query(q),
                        "Query '%s' should work: %s",
                        q.c_str(),
                        mysql_error(test.percona_proxy->conn_rwsplit));
        };

    auto check = [&](string q) {
            ok("START TRANSACTION");
            Row row = get_row(test.percona_proxy->conn_rwsplit, q.c_str());
            ok("COMMIT");
            test.expect(!row.empty() && row[0] == "1", "Query should return 1: %s", q.c_str());
        };

    // Create a table, insert a value and make sure it's replicated to all slaves
    test.percona_proxy->connect_rwsplit();
    ok("CREATE OR REPLACE TABLE test.t1 (id INT)");
    ok("INSERT INTO test.t1 VALUES (1)");
    test.repl->connect();
    test.repl->sync_slaves();
    test.percona_proxy->disconnect();

    cout << "Commit transaction" << endl;
    test.percona_proxy->connect_rwsplit();
    ok("START TRANSACTION");
    ok("SELECT id FROM test.t1 WHERE id = 1 FOR UPDATE");
    switchover();
    ok("UPDATE test.t1 SET id = 2 WHERE id = 1");
    ok("COMMIT");
    check("SELECT COUNT(*) = 1 FROM t1 WHERE id = 2");

    test.percona_proxy->disconnect();

    cout << "Rollback transaction" << endl;
    test.percona_proxy->connect_rwsplit();
    ok("START TRANSACTION");
    ok("UPDATE test.t1 SET id = 1");
    switchover();
    ok("ROLLBACK");
    check("SELECT COUNT(*) = 1 FROM t1 WHERE id = 2");
    test.percona_proxy->disconnect();

    cout << "Read-only transaction" << endl;
    test.percona_proxy->connect_rwsplit();
    ok("START TRANSACTION READ ONLY");
    ok("SELECT @@server_id");   // This causes a checksum mismatch if the transaction is migrated
    switchover();
    ok("COMMIT");
    test.percona_proxy->disconnect();

    test.percona_proxy->connect_rwsplit();
    ok("DROP TABLE test.t1");
    test.percona_proxy->disconnect();

    // Even number of switchovers should bring us back to the original master
    switchover();

    return test.global_result;
}
