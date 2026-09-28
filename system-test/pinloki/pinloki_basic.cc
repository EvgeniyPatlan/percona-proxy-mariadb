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

#include <maxtest/testconnections.hh>
#include "test_base.hh"

class BasicTest : public TestCase
{
public:
    using TestCase::TestCase;

    void pre() override
    {
        // Create a table with one row
        test.expect(master.query("CREATE TABLE test.t1(id INT)"), "CREATE failed: %s", master.error());
        test.expect(master.query("INSERT INTO test.t1 VALUES (1)"), "INSERT failed: %s", master.error());
        test.expect(master.query("FLUSH LOGS"), "FLUSH failed: %s", master.error());
        test.expect(master.query("CREATE TABLE test.t2 (id INT)"), "CREATE failed: %s", master.error());
        test.expect(master.query("INSERT INTO test.t2 VALUES (1)"), "INSERT failed: %s", master.error());
        sync_all();
    }

    void run() override
    {
        // Set the address of "pinloki"-server to the IP of Percona Proxy, so that the address used by
        // MariaDB-Monitor matches the address used by server2. Reconfigure the monitor so that it
        // updates its internal bookkeeping.
        test.percona_proxy->percona_proxyctlf("alter server pinloki address=%s", test.percona_proxy->ip4());
        test.percona_proxy->percona_proxyctl("unlink monitor mariadb-cluster pinloki");
        test.percona_proxy->percona_proxyctl("link monitor mariadb-cluster pinloki");
        test.percona_proxy->wait_for_monitor(1);

        auto servers = test.percona_proxy->get_servers();
        servers.print();
        auto slave_st = mxt::ServerInfo::slave_st;
        servers.check_servers_status({mxt::ServerInfo::master_st, slave_st, slave_st, slave_st,
                                      mxt::ServerInfo::BLR | mxt::ServerInfo::RUNNING});

        // test.t1 should contain one row
        auto result = slave.field("SELECT COUNT(*) FROM test.t1");
        test.expect(result == "1", "`test`.`t1` should have one row.");

        result = slave.field("SELECT COUNT(*) FROM test.t2");
        test.expect(result == "1", "`test`.`t2` should have one row.");

        // All servers should be at the same GTID
        check_gtid();

        // Run the diagnostics function, mainly for code coverage.
        test.check_percona_proxyctl("show services");

        // Some simple sanity checks
        auto rows = percona_proxy.rows("SHOW MASTER STATUS");
        test.expect(!rows.empty(), "SHOW MASTER STATUS should return a resultset");
        test.expect(!percona_proxy.query("This should not break anything"), "Bad SQL should fail");
        test.expect(!percona_proxy.query("CHANGE MASTER 'name' TO MASTER_HOST='localhost'"),
                    "CHANGE MASTER with connection name should fail");

        auto direct = test.repl->backend(2)->admin_connection()->query("SHOW SLAVE STATUS");
        test.expect(direct->next_row(), "Empty direct result");
        auto c = test.percona_proxy->open_rwsplit_connection2();

        const auto variables = {"Master_Log_File", "Read_Master_Log_Pos", "Exec_Master_Log_Pos"};

        if (test.ok())
        {
            bool ok = false;

            for (int i = 0; i < 10 && test.ok(); i++)
            {
                if (auto via_percona_proxy = c->query("SHOW SLAVE STATUS"))
                {
                    test.expect(via_percona_proxy->next_row(), "Empty percona-proxy result");
                    ok = true;

                    for (std::string field : variables)
                    {
                        auto expected = direct->get_string(field);
                        result = via_percona_proxy->get_string(field);

                        if (expected != result)
                        {
                            test.tprintf("Expected %s to be %s but it was %s",
                                         field.c_str(), expected.c_str(), result.c_str());
                            ok = false;
                        }
                    }

                    if (ok)
                    {
                        break;
                    }
                    else
                    {
                        std::this_thread::sleep_for(1s);
                    }
                }
            }

            test.expect(ok, "Binlogrouter should eventually catch up");
        }
    }

    void post() override
    {
        test.expect(master.query("DROP TABLE test.t1"), "DROP failed: %s", master.error());
    }

private:
};

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    return BasicTest(test).result();
}
