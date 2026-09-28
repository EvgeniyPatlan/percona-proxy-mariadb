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

#include <iostream>
#include <string>
#include <vector>
#include <maxtest/testconnections.hh>

using namespace std;

namespace
{

const int PORT_RWS_REDIS = 4006;

void test_that_connecting_fails(TestConnections& test)
{
    test.tprintf("Testing that connecting fails.");

    Connection c = test.percona_proxy->get_connection(PORT_RWS_REDIS);
    test.expect(c.connect(), "1: Could not connect to Percona Proxy.");

    c.query("SELECT 1"); // Trigger connecting to Redis
    sleep(1);
    c.query("SELECT 1");
    sleep(1);

    test.log_includes("NOAUTH Authentication required");
}

void test_that_connecting_succeeds(TestConnections& test)
{
    test.tprintf("Testing that connecting succeeds.");

    Connection c = test.percona_proxy->get_connection(PORT_RWS_REDIS);
    test.expect(c.connect(), "1: Could not connect to Percona Proxy.");

    c.query("SELECT 1"); // Trigger connecting to Redis
    sleep(1);
    c.query("SELECT 1");
    sleep(1);

    test.log_includes("Redis authentication succeeded");
}

}

void install_and_start_redis(mxt::PerconaProxy& percona_proxies)
{
    setenv("percona_proxy_000_keyfile", percona_proxies.sshkey(), 0);
    setenv("percona_proxy_000_whoami", percona_proxies.access_user(), 0);
    setenv("percona_proxy_000_network", percona_proxies.ip4(), 0);

    // This will install memcached as well, but that's ok.

    string path(mxt::SOURCE_DIR);
    path += "/cache_install_and_start_storages.sh";

    system(path.c_str());
}

int main(int argc, char* argv[])
{
    TestConnections::skip_percona_proxy_start(true);
    TestConnections test(argc, argv);

    auto percona_proxies = test.percona_proxy;

    install_and_start_redis(*percona_proxies);

    // Make redis require a password
    percona_proxies->ssh_node(
        "sed -i \"s/# requirepass foobared/requirepass foobared/\" /etc/redis.conf; "
        "systemctl restart redis",
        true);

    percona_proxies->start();
    sleep(1);

    test_that_connecting_fails(test);

    // Make Percona Proxy provide a password
    percona_proxies->ssh_node(
        "sed -i \"s/server=127.0.0.1/server=127.0.0.1,password=foobared/\" /etc/percona-proxy.cnf; "
        "systemctl restart percona-proxy",
        true);

    sleep(1);

    test_that_connecting_succeeds(test);

    // Remove redis requirement of a password
    percona_proxies->ssh_node(
        "sed -i \"s/requirepass foobared/# requirepass foobared/\" /etc/redis.conf; "
        "systemctl restart redis",
        true);

    return test.global_result;
}
