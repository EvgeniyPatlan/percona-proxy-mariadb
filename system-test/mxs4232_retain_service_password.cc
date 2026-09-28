/*
 * Copyright (c) 2023 MariaDB plc
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

#include <maxtest/maxrest.hh>
#include <maxtest/testconnections.hh>

namespace
{

void delete_user(TestConnections& test, Connection& c)
{
    test.expect(c.query("DROP USER IF EXISTS 'mxs4232'@'%'"),
                "Could not drop user: %s", c.error());
}

void create_user(TestConnections& test, Connection& c)
{
    test.expect(c.query("CREATE USER 'mxs4232'@'%' IDENTIFIED by 'mxs4232'"),
                "Could not create user: %s", c.error());

    test.expect(c.query("GRANT ALL PRIVILEGES ON *.* TO 'mxs4232'@'%'"),
                "Could not grant access: %s", c.error());
}

void run(TestConnections& test)
{
    MaxRest maxrest(&test);

    // Change the service password => it's no longer valid as the server
    // still expects the original one, i.e. "skysql".
    maxrest.alter_service("RWS", "password", "non-working-password");

    // Create user directly at master.
    Connection master = test.repl->get_connection(0);
    test.expect(master.connect(), "Could not connect to master: %s", master.error());
    create_user(test, master);

    test.sync_repl_slaves();

    // Connect using new user via Percona Proxy. Unless Percona Proxy uses the previous
    // correct password, the connecting will fail.
    Connection percona_proxy = test.percona_proxy->rwsplit();

    percona_proxy.set_credentials("mxs4232", "mxs4232");

    test.expect(percona_proxy.connect(), "Could not connect to Percona Proxy: %s", percona_proxy.error());

    test.expect(percona_proxy.query("SELECT 1"), "Could not SELECT 1:, %s", percona_proxy.error());
}

}

int main(int argc, char** argv)
{
    TestConnections::skip_percona_proxy_start(true);

    TestConnections test(argc, argv);

    // Delete the user before Percona Proxy has started and loads the users.
    Connection master = test.repl->get_connection(0);
    test.expect(master.connect(), "Could not connect to master: %s", master.error());
    delete_user(test, master);

    test.percona_proxy->start();

    try
    {
        run(test);
    }
    catch (const std::exception& x)
    {
        test.add_failure("Exception: %s", x.what());
    }

    delete_user(test, master);

    return test.global_result;
}
