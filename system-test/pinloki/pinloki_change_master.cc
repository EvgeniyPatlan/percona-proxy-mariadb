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

class ChangeMasterTest : public TestCase
{
public:
    using TestCase::TestCase;

    void pre() override
    {
        master.query("CREATE TABLE test.t1(id INT)");
    }

    void run() override
    {
        for (int i = 0; i < 5 && test.ok(); i++)
        {
            swap_master();
        }
    }


    void post() override
    {
        master.query("DROP TABLE test.t1");
    }

private:
    void swap_master()
    {
        test.tprintf("Check that starting setup works");
        check(master, slave);

        test.tprintf("Stop slave on promoted slave");
        slave.query("STOP SLAVE");
        test.tprintf("Flush logs until the promoted slave is ahead of the master");
        flush_until_ahead(slave, master.field("SHOW MASTER STATUS"));

        test.tprintf("Point Percona Proxy to it");
        percona_proxy.query("STOP SLAVE");
        percona_proxy.query(change_master_sql(test.repl->ip(1), test.repl->port(1)));
        percona_proxy.query("START SLAVE");

        test.tprintf("Point demoted master to percona-proxy");
        master.query(change_master_sql(test.percona_proxy->ip(), test.percona_proxy->rwsplit_port,
                                       GtidPos::CURRENT));
        master.query("START SLAVE");

        test.tprintf("Check that new setup works");
        check(slave, master);

        test.tprintf("Stop slave on demoted master");
        master.query("STOP SLAVE");
        test.tprintf("Flush logs until the demoted master is ahead of the promoted slave");
        flush_until_ahead(master, slave.field("SHOW MASTER STATUS"));

        test.tprintf("Point Percona Proxy to the original master");
        percona_proxy.query("STOP SLAVE");
        percona_proxy.query(change_master_sql(test.repl->ip(0), test.repl->port(0)));
        percona_proxy.query("START SLAVE");

        test.tprintf("Point original slave back at Percona Proxy");
        slave.query(change_master_sql(test.percona_proxy->ip(), test.percona_proxy->rwsplit_port,
                                      GtidPos::CURRENT));
        slave.query("START SLAVE");

        test.tprintf("Check that resulting setup works");
        check(master, slave);
    }

    void check(Connection& m, Connection& s)
    {
        m.query("INSERT INTO test.t1 VALUES (1)");
        sync(m, percona_proxy);
        sync(percona_proxy, s);

        auto master_rows = m.field("SELECT COUNT(*) FROM test.t1");
        auto slave_rows = s.field("SELECT COUNT(*) FROM test.t1");

        test.expect(master_rows == slave_rows,
                    "Expected slave to have %s rows but it was %s",
                    master_rows.c_str(), slave_rows.c_str());

        check_gtid();
    }

    void flush_until_ahead(Connection& c, const std::string& current_binlog)
    {
        int target = atoi(current_binlog.substr(current_binlog.find_last_of('.') + 1).c_str());
        auto binlog = c.field("SHOW MASTER STATUS");

        while (atoi(binlog.substr(binlog.find_last_of('.') + 1).c_str()) <= target)
        {
            c.query("FLUSH LOGS");
            binlog = c.field("SHOW MASTER STATUS");
        }
    }
};

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    return ChangeMasterTest(test).result();
}
