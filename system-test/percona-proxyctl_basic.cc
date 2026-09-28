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
 * Minimal Percona Proxyctl sanity check
 */

#include <maxtest/testconnections.hh>
#include <maxbase/format.hh>

void test_reload_tls(TestConnections& test)
{
    test.percona_proxy->ssh_node_f(true, "rm /var/lib/percona-proxy/percona-proxy.cnf.d/*");

    auto& mxs = *test.percona_proxy;
    std::string key = mxs.cert_key_path();
    std::string cert = mxs.cert_path();
    int rc = test.percona_proxy->ssh_node_f(true, "sed -i "
                                             " -e '/percona-proxy/ a admin_ssl_key=%s'"
                                             " -e '/percona-proxy/ a admin_ssl_cert=%s'"
                                             " /etc/percona-proxy.cnf", key.c_str(), cert.c_str());
    test.expect(rc == 0, "Failed to enable encryption for the REST API");
    test.percona_proxy->restart();

    test.tprintf("TLS reload sanity check");
    test.expect(test.percona_proxyctl("-s -n false list servers").rc == 0, "`list servers` should work");
    test.expect(test.percona_proxyctl("list servers").rc != 0, "Command without --secure should fail");

    test.expect(test.percona_proxyctl("-s -n false reload tls").rc == 0, "`reload tls` should work");
    test.expect(test.percona_proxyctl("-s -n false list servers").rc == 0, "`list servers` should work");

    // Need to copy the client certificate and key to Percona Proxy node.
    const char* home = mxs.access_homedir();
    std::string client_cert_src = mxb::string_printf("%s/ssl-cert/client.crt", mxt::SOURCE_DIR);
    std::string client_key_src = mxb::string_printf("%s/ssl-cert/client.key", mxt::SOURCE_DIR);
    std::string client_cert_dst = mxb::string_printf("%s/certs/extra-key.pem", home);
    std::string client_key_dst = mxb::string_printf("%s/certs/extra-cert.pem", home);
    mxs.copy_to_node(client_cert_src, client_cert_dst);
    mxs.copy_to_node(client_key_src, client_key_dst);

    test.tprintf("MXS-4968: REST-API TLS certs can be reloaded but not modified");
    auto cmd = mxb::string_printf("-s -n false alter percona-proxy admin_ssl_key=%s admin_ssl_cert=%s",
                                  client_key_dst.c_str(), client_cert_dst.c_str());
    test.check_percona_proxyctl(cmd);
    test.check_percona_proxyctl("-s -n false list servers");

    // This tests that invalid paths are handled correctly. This covers a case where a debug assertion is hit
    // if an invalid path was given to ParamPath.
    test.percona_proxyctl("-s -n false alter percona-proxy admin_ssl_key=/foo/bar.cert admin_ssl_cert=/foo/bar.key");

    cmd = mxb::string_printf("-s -n false alter percona-proxy admin_ssl_key=%s "
                             "admin_ssl_cert=%s", key.c_str(), cert.c_str());
    test.check_percona_proxyctl(cmd);
    // Delete the copied files.
    mxs.vm_node().delete_from_node(client_cert_dst);
    mxs.vm_node().delete_from_node(client_key_dst);

    test.check_percona_proxyctl("-s -n false list servers");

    if (test.ok())
    {
        test.tprintf("TLS reload stress test");
        std::vector<std::thread> threads;
        std::atomic<bool> running {true};

        for (int i = 0; i < 10; i++)
        {
            threads.emplace_back([&](){
                int num = 0;
                while (running)
                {
                    ++num;
                    auto res = test.percona_proxy->ssh_output("percona-proxyctl -s -n false list servers", false);
                    test.expect(res.rc == 0, "`list servers` should not fail: %d, %s", res.rc,
                                res.output.c_str());
                }

                test.tprintf("Executed %d commands", num);
            });
        }

        for (int i = 0; i < 20; i++)
        {
            auto res = test.percona_proxyctl("-s -n false reload tls");
            test.expect(res.rc == 0, "`reload tls` should work: %d, %s", res.rc, res.output.c_str());
        }

        running = false;

        for (auto& t : threads)
        {
            t.join();
        }
    }

    test.percona_proxy->ssh_node_f(true, "sed -i  -e '/admin_ssl/ d' /etc/percona-proxy.cnf");
    test.percona_proxy->ssh_node_f(true, "rm /var/lib/percona-proxy/percona-proxy.cnf.d/percona-proxy.cnf");
    test.percona_proxy->restart();
}

void test_cert_chain(TestConnections& test)
{
    test.percona_proxy->ssh_node_f(true, "rm /var/lib/percona-proxy/percona-proxy.cnf.d/*");
    const char* home = test.percona_proxy->access_homedir();
    std::string key = test.percona_proxy->cert_key_path();
    std::string cert = test.percona_proxy->cert_path();
    std::string ca_cert = test.percona_proxy->ca_cert_path();

    int rc = test.percona_proxy->ssh_node_f(
        true, "cat %s %s > %s/certs/server-chain-cert.pem",
        cert.c_str(), ca_cert.c_str(), home);
    test.expect(rc == 0, "Failed to combine certificates into a chain");

    rc = test.percona_proxy->ssh_node_f(
        true,
        "sed -i "
        " -e '/percona-proxy/ a admin_ssl_key=%s'"
        " -e '/percona-proxy/ a admin_ssl_cert=%s/certs/server-chain-cert.pem'"
        " /etc/percona-proxy.cnf", key.c_str(), home);
    test.expect(rc == 0, "Failed to enable encryption for the REST API");
    test.percona_proxy->restart();

    test.expect(test.percona_proxyctl("-s -n false list servers").rc == 0, "`list servers` should work");
    test.expect(test.percona_proxyctl("list servers").rc != 0, "Command without --secure should fail");
    test.expect(test.percona_proxyctl("-s -n false reload tls").rc == 0, "`reload tls` should work");
    test.expect(test.percona_proxyctl("-s -n false list servers").rc == 0, "`list servers` should work after reload");

    test.percona_proxy->ssh_node_f(true, "sed -i  -e '/admin_ssl/ d' /etc/percona-proxy.cnf");
    test.percona_proxy->restart();
}

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);

    // Create one connection so that the sessions endpoint has some content
    auto c = test.percona_proxy->rwsplit();
    c.connect();

    int rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl --help list servers");
    test.expect(rc == 0, "`--help list servers` should work");

    rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl --tsv list servers|grep 'Master, Running'");
    test.expect(rc == 0, "`list servers` should return at least one row with: Master, Running");

    rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl set server server2 maintenance");
    test.expect(rc == 0, "`set server` should work");

    rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl --tsv list servers|grep 'Maintenance'");
    test.expect(rc == 0, "`list servers` should return at least one row with: Maintanance");

    rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl clear server server2 maintenance");
    test.expect(rc == 0, "`clear server` should work");

    rc = test.percona_proxy->ssh_node_f(false, "percona-proxyctl --tsv list servers|grep 'Maintenance'");
    test.expect(rc != 0, "`list servers` should have no rows with: Maintanance");

    test.tprintf("Execute all available commands");
    test.percona_proxy->ssh_node_f(false,
                              "percona-proxyctl list servers;"
                              "percona-proxyctl list services;"
                              "percona-proxyctl list listeners RW-Split-Router;"
                              "percona-proxyctl list monitors;"
                              "percona-proxyctl list sessions;"
                              "percona-proxyctl list filters;"
                              "percona-proxyctl list modules;"
                              "percona-proxyctl list threads;"
                              "percona-proxyctl list users;"
                              "percona-proxyctl list commands;"
                              "percona-proxyctl show server server1;"
                              "percona-proxyctl show servers;"
                              "percona-proxyctl show service RW-Split-Router;"
                              "percona-proxyctl show services;"
                              "percona-proxyctl show monitor MySQL-Monitor;"
                              "percona-proxyctl show monitors;"
                              "percona-proxyctl show session 1;"
                              "percona-proxyctl show sessions;"
                              "percona-proxyctl show filter qla;"
                              "percona-proxyctl show filters;"
                              "percona-proxyctl show module readwritesplit;"
                              "percona-proxyctl show modules;"
                              "percona-proxyctl show percona-proxy;"
                              "percona-proxyctl show thread 1;"
                              "percona-proxyctl show threads;"
                              "percona-proxyctl show logging;"
                              "percona-proxyctl show commands mariadbmon;"
                              "percona-proxyctl clear server server1 maintenance;"
                              "percona-proxyctl enable log-priority info;"
                              "percona-proxyctl disable log-priority info;"
                              "percona-proxyctl create server server5 127.0.0.1 3306;"
                              "percona-proxyctl create monitor mon1 mariadbmon user=skysql password=skysql;"
                              "percona-proxyctl create service svc1 readwritesplit user=skysql password=skysql;"
                              "percona-proxyctl create filter qla2 qlafilter filebase=/tmp/qla2.log;"
                              "percona-proxyctl create listener svc1 listener1 9999;"
                              "percona-proxyctl create user maxuser maxpwd;"
                              "percona-proxyctl link service svc1 server5;"
                              "percona-proxyctl link monitor mon1 server5;"
                              "percona-proxyctl alter service-filters svc1 qla2;"
                              "percona-proxyctl unlink service svc1 server5;"
                              "percona-proxyctl unlink monitor mon1 server5;"
                              "percona-proxyctl alter service-filters svc1"
                              "percona-proxyctl destroy server server5;"
                              "percona-proxyctl destroy listener svc1 listener1;"
                              "percona-proxyctl destroy monitor mon1;"
                              "percona-proxyctl destroy filter qla2;"
                              "percona-proxyctl destroy service svc1;"
                              "percona-proxyctl destroy user maxuser;"
                              "percona-proxyctl stop service RW-Split-Router;"
                              "percona-proxyctl stop monitor MySQL-Monitor;"
                              "percona-proxyctl stop percona-proxy;"
                              "percona-proxyctl start service RW-Split-Router;"
                              "percona-proxyctl start monitor MySQL-Monitor;"
                              "percona-proxyctl start percona-proxy;"
                              "percona-proxyctl alter server server1 port 3307;"
                              "percona-proxyctl alter server server1 port 3306;"
                              "percona-proxyctl alter monitor MySQL-Monitor auto_failover true;"
                              "percona-proxyctl alter service RW-Split-Router max_slave_connections=3;"
                              "percona-proxyctl alter service RW-Split-Router slave_selection_criteria=adaptive_routing;"
                              "percona-proxyctl alter logging ms_timestamp true;"
                              "percona-proxyctl alter percona-proxy passive true;"
                              "percona-proxyctl rotate logs;"
                              "percona-proxyctl call command mariadbmon reset-replication MySQL-Monitor;"
                              "percona-proxyctl api get servers;"
                              "percona-proxyctl classify 'select 1';"
                              "percona-proxyctl debug stacktrace;"
                              "percona-proxyctl debug stacktrace --raw;"
                              "percona-proxyctl debug stacktrace --fold;"
                              "percona-proxyctl debug stacktrace --duration=1;"
                              "percona-proxyctl debug stacktrace --duration=1 --interval=100;"
                              "percona-proxyctl --timeout 30s create report test-report.txt"
                              );

    test.tprintf("MXS-3697: Percona Proxyctl fails with \"ENOENT: no such file or directory, stat '/~/.percona-proxyctl.cnf'\" "
                 "when running commands from the root directory.");
    rc = test.percona_proxy->ssh_node_f(false, "cd / && percona-proxyctl list servers");
    test.expect(rc == 0, "Failed to execute a command from the root directory");

    test.tprintf("MXS-4169: Listeners wrongly require ssl_ca_cert when created at runtime");
    test.check_percona_proxyctl("create service my-test-service readconnroute user=maxskysql password=skysql");
    std::string key = test.percona_proxy->cert_key_path();
    std::string cert = test.percona_proxy->cert_path();
    test.check_percona_proxyctl(
        "create listener my-test-service my-test-listener 6789 ssl=true "
        "ssl_key=" + key + " ssl_cert=" + cert);
    test.check_percona_proxyctl("destroy listener my-test-listener");
    test.check_percona_proxyctl("destroy service my-test-service");

    // Also checks that Percona Proxyctl works correctly when the REST API uses encryption.
    test.tprintf("MXS-4041: Reloading of REST API TLS certificates");
    test_reload_tls(test);

    test.tprintf("MXS-4442: TLS certificate chain in admin_ssl_cert");
    test_cert_chain(test);

    test.tprintf("MXS-4171: Runtime modifications to static parameters");

    auto res = test.percona_proxyctl("alter service RW-Split-Router router=readwritesplit");
    test.expect(res.rc == 0, "Changing `router` to its current value should succeed: %s", res.output.c_str());

    res = test.percona_proxyctl("alter service RW-Split-Router router=readconnroute");
    test.expect(res.rc != 0, "Changing `router` to a new value should fail.");

    res = test.percona_proxyctl("alter listener RW-Split-Listener protocol=MySQLClient");
    test.expect(res.rc == 0, "Old alias for module name should compare equal: %s", res.output.c_str());

    res = test.percona_proxyctl("alter listener RW-Split-Listener protocol=cdc");
    test.expect(res.rc != 0, "Changing listener protocol should fail.");

    // MXS-5126: Crash in cache filter with 'percona-proxyctl show filters'
    c.connect();
    test.check_percona_proxyctl("create filter CacheFilter cache storage=storage_inmemory");
    test.check_percona_proxyctl("alter service-filters RW-Split-Router CacheFilter");

    for (int i = 0; i < 10; i++)
    {
        c.query("SELECT 1");
    }

    test.check_percona_proxyctl("show filters");

    test.tprintf("MXS-5947: 'create report' broken with maxlog=false syslog=true");
    test.check_percona_proxyctl("alter percona-proxy syslog=true maxlog=true");
    test.check_percona_proxyctl("create report");
    test.check_percona_proxyctl("create report /tmp/percona-proxyctl-report.txt");
    test.check_percona_proxyctl("create report --archive /tmp/percona-proxyctl-report.tar");
    test.log_includes("Failed to read any data from the systemd journal");

    test.check_percona_proxy_alive();
    return test.global_result;
}
