/*
 * Copyright (c) 2024 MariaDB plc, Finnish Branch
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

using std::string;

namespace
{
void test_main(TestConnections& test)
{
    auto& mxs = *test.percona_proxy;

    // Start Percona Proxy in a shell in a separate thread.
    std::thread percona_proxy_thread;

    auto run_percona_proxy = [&](bool secure_gui) {
        // Give environment variables in the command. Disable ASAN leak detection as it fails due to internal
        // error, causing Percona Proxy to return an error value.
        auto res = mxs.vm_node().run_cmd_output_sudof(
            "monitor_servers=server1,server2 monitor_user=maxskysql monitor_password=skysql secure_gui=%s "
            "ASAN_OPTIONS=detect_leaks=0 "
            "percona-proxy -d --user=percona-proxy --piddir=/tmp", secure_gui ? "true" : "false");
        if (res.rc == 0)
        {
            test.tprintf("Percona Proxy process exited with code 0.");
        }
        else
        {
            test.add_failure("Percona Proxy exited with error %i. Output: %s", res.rc, res.output.c_str());
        }
    };

    auto start_percona_proxy = [&](bool secure_gui) {
        test.tprintf("Starting Percona Proxy.");
        percona_proxy_thread = std::thread(run_percona_proxy, secure_gui);
        sleep(1);
        mxs.expect_running_status(true);
    };

    auto stop_percona_proxy = [&]() {
        test.tprintf("Shutting down Percona Proxy with kill.");
        mxs.vm_node().run_cmd_output_sudof("kill $(pidof percona-proxy)");
        sleep(1);
        mxs.expect_running_status(false);
        percona_proxy_thread.join();
    };

    start_percona_proxy(true);

    auto servers = mxs.get_servers();
    mxs.check_print_servers_status({mxt::ServerInfo::master_st, mxt::ServerInfo::slave_st,
                                    mxt::ServerInfo::DOWN, mxt::ServerInfo::DOWN});

    if (test.ok())
    {
        test.tprintf("Environment variable substitution works.");

        test.tprintf("Testing admin_secure_gui=true, fetching GUI should give a message.");

        const string curl_fetch_gui = "curl --silent -u admin:mariadb http://localhost:8989";
        const string insecure_gui = "The Percona Proxy GUI requires HTTPS to work, "
                                    "please enable it by configuring";

        auto res = mxs.vm_node().run_cmd_output_sudo(curl_fetch_gui);
        if (res.rc == 0)
        {
            test.expect(res.output.find(insecure_gui) != string::npos, "Did not find the expected message.");
            test.expect(res.output.size() < 5000, "Unexpected output length.");
            if (test.ok())
            {
                test.tprintf("Received message explaining GUI is insecure.");
            }
        }
        else
        {
            test.add_failure("curl failed. Error %i, %s", res.rc, res.output.c_str());
        }

        if (test.ok())
        {
            stop_percona_proxy();

            test.tprintf("Testing admin_secure_gui=false, fetching GUI should work.");
            start_percona_proxy(false);

            res = mxs.vm_node().run_cmd_output_sudo(curl_fetch_gui);
            if (res.rc == 0)
            {
                test.expect(res.output.find(insecure_gui, 0) == string::npos,
                            "Found message when expecting GUI.");
                test.expect(res.output.size() > 5000, "Unexpected output length.");
                if (test.ok())
                {
                    test.tprintf("Received the GUI page.");
                }
            }
            else
            {
                test.add_failure("curl failed. Error %i, %s", res.rc, res.output.c_str());
            }
        }
    }

    stop_percona_proxy();
}
}

int main(int argc, char* argv[])
{
    TestConnections test;
    TestConnections::skip_percona_proxy_start(true);
    return test.run_test(argc, argv, test_main);
}
