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

using namespace std;

namespace
{

const int PORT_RWS_REDIS = 4006;

bool install_dependency(TestConnections& test, const char* zDependency)
{
    test.tprintf("Installing %s.", zDependency);

    int rv = test.percona_proxy->ssh_node_f(true, "yum install -y %s", zDependency);
    test.expect(rv == 0, "Could not install %s.", zDependency);

    return rv == 0;
}

bool build_redis(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    int rv = 0;

    // Try to enable EPEL repositories if possible. If not, don't treat it as an error. This way the
    // installation will only fail if the other dependencies can't be installed.
    // TODO: The dependencies are RHEL-specific and prevent this test from being run on a non-RHEL machine.
    install_dependency(test, "epel-release");
    rv += !install_dependency(test, "git");
    rv += !install_dependency(test, "make");
    rv += !install_dependency(test, "gcc");
    rv += !install_dependency(test, "jemalloc-devel");
    rv += !install_dependency(test, "openssl-devel");

    if (rv == 0)
    {
        test.tprintf("Removing possible old redis installation.");
        rv = percona_proxy->ssh_node_f(false, "cd %s; rm -rf redis",
                                  percona_proxy->access_homedir());
        test.expect(rv == 0, "Could not remove old redis installation.");

        if (rv == 0)
        {
            test.tprintf("Cloning redis.");
            rv = percona_proxy->ssh_node_f(false, "cd %s; git clone https://github.com/redis/redis.git",
                                      percona_proxy->access_homedir());
            test.expect(rv == 0, "Could not clone redis.");

            if (rv == 0)
            {
                test.tprintf("Checking out 6.2.8.");
                rv = percona_proxy->ssh_node_f(false, "cd %s/redis; git checkout 6.2.8",
                                          percona_proxy->access_homedir());
                test.expect(rv == 0, "Could not checkout 6.2.8.");

                if (rv == 0)
                {
                    test.tprintf("Building redis.");
                    rv = percona_proxy->ssh_node_f(false, "cd %s/redis; make BUILD_TLS=yes",
                                              percona_proxy->access_homedir());
                    test.expect(rv == 0, "Could not build redis.");
                }
            }
        }
    }

    return rv == 0;
}

bool generate_certificates(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    int rv;

    const char* zHome = percona_proxy->access_homedir();

    test.tprintf("Generating certificates.");
    rv = percona_proxy->ssh_node_f(false, "cd %s/redis; ./utils/gen-test-certs.sh", zHome);
    test.expect(rv == 0, "Could not generate certificates.");

    rv = percona_proxy->ssh_node_f(true,
                              "chmod o+x %s/redis;"
                              "chmod o+x %s/redis/tests;"
                              "chmod o+x %s/redis/tests/tls;"
                              "chmod o+r %s/redis/tests/tls/*",
                              zHome, zHome, zHome, zHome);
    test.expect(rv == 0, "Could not change mode on files.");

    return rv == 0;
}

bool stop_system_redis(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    int rv;

    test.tprintf("Stopping system redis.");
    rv = percona_proxy->ssh_node_f(true, "systemctl stop redis");
    test.expect(rv == 0, "Could not stop system redis.");

    return rv == 0;
}

bool start_custom_redis(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    int rv;

    test.tprintf("Starting custom redis.");
    rv = percona_proxy->ssh_node_f(false,
                              "cd %s/redis; "
                              "./src/redis-server --daemonize yes --tls-port 6379 --port 0 "
                              "--tls-cert-file ./tests/tls/redis.crt "
                              "--tls-key-file ./tests/tls/redis.key "
                              "--tls-ca-cert-file ./tests/tls/ca.crt",
                              percona_proxy->access_homedir());
    test.expect(rv == 0, "Could not start custom redis.");

    return rv == 0;
}

bool stop_custom_redis(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    int rv;

    test.tprintf("Stopping custom redis.");
    rv = percona_proxy->ssh_node_f(true, "pkill redis-server");
    test.expect(rv == 0, "Could not stop custom redis.");

    return rv == 0;
}

void test_that_usage_fails(TestConnections& test)
{
    test.tprintf("Testing that usage fails.");

    Connection c = test.percona_proxy->get_connection(PORT_RWS_REDIS);
    test.expect(c.connect(), "Could not connect to Percona Proxy.");

    c.query("SELECT 1"); // Trigger connecting to Redis
    sleep(1);
    c.query("SELECT 1");
    sleep(1);

    test.log_includes("I/O-error; will attempt to reconnect");
}

void test_that_usage_succeeds(TestConnections& test)
{
    test.tprintf("Testing that usage succeeds.");

    Connection c = test.percona_proxy->get_connection(PORT_RWS_REDIS);
    test.expect(c.connect(), "Could not connect to Percona Proxy.");

    c.query("SELECT 1"); // Trigger connecting to Redis
    sleep(1);
    c.query("SELECT 1");
    sleep(1);

    test.log_excludes("I/O-error; will attempt to reconnect");
}

void run_test(TestConnections& test)
{
    auto percona_proxy = test.percona_proxy;

    test.expect(percona_proxy->start_and_check_started(), "Could not start percona-proxy.");
    test_that_usage_fails(test);

    test.expect(percona_proxy->stop_and_check_stopped(), "Could not stop percona-proxy.");

    test.tprintf("Configuring Percona Proxy for SSL.");
    int rv = percona_proxy->ssh_node_f(
        true,
        "sed -i "
        "-e \"s@storage_redis.ssl=false@storage_redis.ssl=true@\" "
        "-e \"s@storage_redis.ssl_cert=/etc/percona-proxy.cnf@storage_redis.ssl_cert=%s/redis/tests/tls/redis.crt@\" "
        "-e \"s@storage_redis.ssl_key=/etc/percona-proxy.cnf@storage_redis.ssl_key=%s/redis/tests/tls/redis.key@\" "
        "-e \"s@storage_redis.ssl_ca=/etc/percona-proxy.cnf@storage_redis.ssl_ca=%s/redis/tests/tls/ca.crt@\" "
        "/etc/percona-proxy.cnf",
        percona_proxy->access_homedir(),
        percona_proxy->access_homedir(),
        percona_proxy->access_homedir());

    test.expect(rv == 0, "Could not configure Percona Proxy for SSL.");

    if (rv == 0)
    {
        rv = percona_proxy->ssh_node_f(true, "rm /var/log/percona-proxy/percona-proxy.log");
        test.expect(rv == 0, "Could not remove /var/log/percona-proxy/percona-proxy.log");

        test.expect(percona_proxy->start_and_check_started(), "Could not start percona-proxy.");
        test_that_usage_succeeds(test);
    }
}

}

int main(int argc, char* argv[])
{
    TestConnections::skip_percona_proxy_start(true);
    TestConnections test(argc, argv);

    if (build_redis(test))
    {
        test.tprintf("Redis built.");

        if (generate_certificates(test))
        {
            test.tprintf("Certificates generated.");

            if (stop_system_redis(test))
            {
                test.tprintf("System redis stopped.");

                if (start_custom_redis(test))
                {
                    test.tprintf("Custom redis started.");

                    run_test(test);

                    if (stop_custom_redis(test))
                    {
                        test.tprintf("Custom redis stopped.");
                    }
                }
            }
        }
    }

    return test.global_result;
}
