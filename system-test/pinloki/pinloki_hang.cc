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

class HangTest : public TestCase
{
public:
    using TestCase::TestCase;

    void run() override
    {
        std::thread thr(&HangTest::change_master, this);

        for (int i = 0; i < 50 && test.ok(); i++)
        {
            test.check_percona_proxyctl("show threads");
        }

        m_running = false;
        thr.join();
    }


private:
    void change_master()
    {
        percona_proxy.set_timeout(10);
        percona_proxy.connect();

        while (m_running && test.ok())
        {
            test.expect(percona_proxy.query("STOP SLAVE"),
                        "STOP SLAVE failed: %s", percona_proxy.error());
            test.expect(percona_proxy.query(change_master_sql(test.repl->ip(0), test.repl->port(0))),
                        "CHANGE MASTER failed: %s", percona_proxy.error());
            test.expect(percona_proxy.query("START SLAVE"),
                        "START SLAVE failed: %s", percona_proxy.error());
        }
    }

    std::atomic<bool> m_running {true};
};

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    return HangTest(test).result();
}
