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
#include "pinloki_select_master.hh"

int main(int argc, char** argv)
{
    TestConnections::skip_percona_proxy_start(true);
    TestConnections test(argc, argv);

    // Create new encryption keys
    auto rv = test.percona_proxy->ssh_output("percona-proxy-keys");
    test.expect(rv.rc == 0, "percona-proxy-keys failed: %s", rv.output.c_str());

    // Encrypt the password
    rv = test.percona_proxy->ssh_output("percona-proxy-passwd skysql");
    test.expect(rv.rc == 0, "percona-proxy-passwd failed: %s", rv.output.c_str());

    // Replace the passwords with the encrypted ones
    test.percona_proxy->ssh_output(
        "sed -i 's/password=wrong_password/password=" + rv.output + "/' /etc/percona-proxy.cnf");
    test.percona_proxy->start();
    test.percona_proxy->wait_for_monitor(2);

    return MasterSelectTest(test).result();
}
