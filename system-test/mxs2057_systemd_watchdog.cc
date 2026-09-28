/*
 * Copyright (c) 2018 MariaDB Corporation Ab
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
#include <maxbase/stopwatch.hh>

namespace
{
// watchdog_interval 60 seconds, make sure it is the same in percona-proxy.service
const maxbase::Duration watchdog_interval = mxb::from_secs(60.0);

// Return true if percona-proxy stays alive for the duration dur.
bool staying_alive(TestConnections& test, const maxbase::Duration& dur)
{
    bool alive = true;
    maxbase::StopWatch sw_loop_start;
    while (alive && sw_loop_start.split() < dur)
    {
        if (execute_query_silent(test.percona_proxy->conn_rwsplit, "select 1"))
        {
            alive = false;
            break;
        }
    }

    return alive;
}

// The bulk of the test.
void test_watchdog(TestConnections& test, int argc, char* argv[])
{
    test.log_includes("The systemd watchdog is Enabled");

    test.log_printf("Wait for one watchdog interval, systemd should have been notified in that time");
    staying_alive(test, watchdog_interval);

    test.reset_timeout();

    test.log_printf("Make the first thread sleep for 24 hours");
    auto res = test.percona_proxyctl("api get percona-proxy/debug/hang");

    if (res.rc != 0)
    {
        test.tprintf("Call to percona-proxy/debug/hang failed, skipping test as this "
                     "is most likely a release build: %s", res.output.c_str());
        return;
    }

    test.log_printf("Percona Proxy should get killed by systemd in less than duration(interval - epsilon).");
    bool percona_proxy_alive = staying_alive(test, mxb::from_secs(2 * mxb::to_secs(watchdog_interval)));

    if (percona_proxy_alive)
    {
        test.add_result(true, "Although the systemd watchdog is enabled, "
                              "systemd did not terminate percona-proxy!");
    }
    else
    {
        test.log_includes("received fatal signal 6");
        if (test.global_result == 0)
        {
            test.tprintf("Maxscale was killed by systemd - ok");

            for (int i = 0; i < 30; i++)
            {
                if (test.percona_proxy->ssh_output("rm /tmp/core*", true).rc == 0)
                {
                    break;
                }
            }

            // Replace the 'fatal signal' log line so that it doesn't trigger a test failure
            test.percona_proxy->ssh_node_f(true, "sed -i 's/fatal signal/REDACTED/' "
                                            "/var/log/percona-proxy/percona-proxy.log");
        }
    }
}
}

int main(int argc, char* argv[])
{
    TestConnections test {argc, argv};
    test.percona_proxy->leak_check(false);
    test.percona_proxy->connect_rwsplit();

    if (!test.global_result)
    {
        test_watchdog(test, argc, argv);
    }

    return test.global_result;
}
