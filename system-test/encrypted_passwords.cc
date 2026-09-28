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
 * @file encrypted_passwords.cpp - Test percona-proxy-keys and percona-proxy-passwd interaction with Percona Proxy
 * - put encrypted password into percona-proxy.cnf and try to use Maxscale
 */

#include <iostream>
#include <maxtest/testconnections.hh>

/** Remove old keys and create a new one */
void create_key(TestConnections& test)
{
    int res = 0;
    test.tprintf("Creating new encryption keys");
    test.percona_proxy->ssh_node(
        "test -f /var/lib/percona-proxy/.secrets && sudo rm /var/lib/percona-proxy/.secrets",
        true);
    test.percona_proxy->ssh_node("percona-proxy-keys", true);
    auto result = test.percona_proxy->ssh_output("sudo test -f /var/lib/percona-proxy/.secrets && echo SUCCESS",
                                            false);

    test.expect(result.output == "SUCCESS", "/var/lib/percona-proxy/.secrets was not created");
    test.percona_proxy->ssh_node("sudo chown percona-proxy:percona-proxy /var/lib/percona-proxy/.secrets", true);
}


/** Hash a new password and start Percona Proxy */
void hash_password(TestConnections& test)
{
    test.percona_proxy->stop();

    test.tprintf("Creating a new encrypted password");
    auto res = test.percona_proxy->ssh_output("percona-proxy-passwd /var/lib/percona-proxy/ skysql");

    std::string enc_pw = res.output;
    auto pos = enc_pw.find('\n');
    if (pos != std::string::npos)
    {
        enc_pw = enc_pw.substr(0, pos);
    }

    test.tprintf("Encrypted password is: %s", enc_pw.c_str());
    test.percona_proxy->ssh_node_f(true,
                              "sed -i -e 's/password[[:space:]]*=[[:space:]]*skysql/password=%s/' /etc/percona-proxy.cnf",
                              enc_pw.c_str());

    test.tprintf("Starting Percona Proxy");
    test.percona_proxy->start_percona_proxy();

    test.tprintf("Checking if Percona Proxy is alive");
    test.expect(test.check_percona_proxy_alive() == 0, "Percona Proxy is not alive");
}

void encrypted_password_in_percona_proxyctl(TestConnections& test)
{
    auto command = [&](std::string cmd, bool ok){
        int rc = test.percona_proxy->ssh_node_f(false, "%s", cmd.c_str());
        test.expect((rc == 0) == ok, "Command %s: %s", ok ? "failed" : "succeeded", cmd.c_str());
    };

    auto command_ok = [&](std::string cmd){
        command(cmd, true);
    };

    auto command_err = [&](std::string cmd){
        command(cmd, false);
    };

    test.tprintf("MXS-5449: Encrypted passwords in Percona Proxyctl");

    const char* user = test.percona_proxy->access_user();
    const char* secretsdir = test.percona_proxy->access_homedir();

    test.percona_proxy->ssh_node_f(true,
                              "cp /var/lib/percona-proxy/.secrets %s/.secrets;"
                              "chown %s %s/.secrets",
                              secretsdir, user, secretsdir);

    test.percona_proxyctl("create user foobar foobar");
    auto enc = test.percona_proxy->ssh_output("percona-proxy-passwd "s + secretsdir + " foobar").output;

    test.percona_proxy->ssh_node_f(true,
                              "echo '[percona-proxyctl]' > /tmp/percona-proxyctl-plaintext.cnf;"
                              "echo 'user=foobar' >> /tmp/percona-proxyctl-plaintext.cnf;"
                              "echo 'password=foobar' >> /tmp/percona-proxyctl-plaintext.cnf;"
                              "echo '[percona-proxyctl]' > /tmp/percona-proxyctl-encrypted.cnf;"
                              "echo 'user=foobar' >> /tmp/percona-proxyctl-encrypted.cnf;"
                              "echo 'password=%s' >> /tmp/percona-proxyctl-encrypted.cnf;"
                              "echo 'secretsdir=%s' >> /tmp/percona-proxyctl-encrypted.cnf;"
                              "chmod 0600 /tmp/percona-proxyctl-plaintext.cnf /tmp/percona-proxyctl-encrypted.cnf;"
                              "chown %s /tmp/percona-proxyctl-plaintext.cnf /tmp/percona-proxyctl-encrypted.cnf;",
                              enc.c_str(), secretsdir, user);

    command_ok("percona-proxyctl --user=foobar --password=foobar list sessions");
    command_ok("percona-proxyctl -c /tmp/percona-proxyctl-plaintext.cnf list sessions");
    command_ok("sudo percona-proxyctl --user=foobar --password=" + enc + " list sessions");
    command_ok("MAXCTRL_USER=foobar MAXCTRL_PASSWORD=foobar percona-proxyctl list sessions");
    command_err("MAXCTRL_USER=wrong MAXCTRL_PASSWORD=wrong percona-proxyctl list sessions");
    command_ok("percona-proxyctl --user=foobar --password=" + enc + " --secretsdir=" + secretsdir + " list sessions");
    command_ok(
        "echo " + enc + "|percona-proxyctl --user=foobar --password='' --secretsdir=" + secretsdir + " list sessions");
    command_ok("percona-proxyctl -c /tmp/percona-proxyctl-encrypted.cnf list sessions");

    test.percona_proxy->ssh_node_f(true, "rm %s/.secrets", secretsdir);

    command_ok("percona-proxyctl --user=foobar --password=foobar list sessions");
    command_ok("percona-proxyctl -c /tmp/percona-proxyctl-plaintext.cnf list sessions");
    command_ok("sudo percona-proxyctl --user=foobar --password=" + enc + " list sessions");
    command_ok("MAXCTRL_USER=foobar MAXCTRL_PASSWORD=foobar percona-proxyctl list sessions");
    command_err("MAXCTRL_USER=wrong MAXCTRL_PASSWORD=wrong percona-proxyctl list sessions");
    command_err("percona-proxyctl --user=foobar --password=" + enc + " --secretsdir=" + secretsdir + " list sessions");
    command_err(
        "echo " + enc + "|percona-proxyctl --user=foobar --password='' --secretsdir=" + secretsdir + " list sessions");
    command_err("percona-proxyctl -c /tmp/percona-proxyctl-encrypted.cnf list sessions");

    test.percona_proxyctl("destroy user foobar");
    test.percona_proxy->ssh_node_f(true, "rm /tmp/percona-proxyctl-plaintext.cnf /tmp/percona-proxyctl-encrypted.cnf");
}

void mxs5520_no_password_reencryption(TestConnections& test)
{
    test.tprintf("MXS-5520: Passwords end up being re-encrypted when persisted");

    std::string config_path = "/var/lib/percona-proxy/percona-proxy.cnf.d/percona-proxy.cnf";
    auto res = test.percona_proxy->ssh_output("percona-proxy-passwd /var/lib/percona-proxy/ skysql");
    std::string original_pw = res.output;
    test.percona_proxy->stop();
    test.percona_proxy->ssh_node_f(true, "sed -i \"/percona-proxy/ a config_sync_password=%s\" /etc/percona-proxy.cnf",
                              res.output.c_str());

    test.percona_proxy->start();

    for (int i = 0; i < 5; i++)
    {
        // Do a config change and restart Percona Proxy. This would trigger the re-encryption of
        // an already encrypted password.
        test.percona_proxy->restart();
        test.check_percona_proxyctl("alter percona-proxy passive=true");
        test.check_percona_proxyctl("alter percona-proxy passive=false");

        res = test.percona_proxy->ssh_output("grep config_sync_password " + config_path
                                        + " |cut -f2 -d=");

        test.expect(original_pw == res.output,
                    "Iteration %d: Password in persisted config file is different: %s",
                    i, res.output.c_str());
    }
}

void test_main(TestConnections& test)
{
    create_key(test);
    hash_password(test);
    encrypted_password_in_percona_proxyctl(test);
    mxs5520_no_password_reencryption(test);
}

int main(int argc, char* argv[])
{
    return TestConnections().run_test(argc, argv, test_main);
}
