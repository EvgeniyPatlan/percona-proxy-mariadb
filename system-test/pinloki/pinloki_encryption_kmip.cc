/*
 * Copyright (c) 2022 MariaDB Corporation Ab
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

#include "pinloki_encryption.hh"

int main(int argc, char** argv)
{
    TestConnections::skip_percona_proxy_start(true);
    TestConnections test(argc, argv);

    test.percona_proxy->copy_to_node(mxt::SOURCE_DIR + "/pinloki/install_pykmip.sh"s, "~/install_pykmip.sh");
    test.percona_proxy->copy_to_node(mxt::SOURCE_DIR + "/pinloki/start_pykmip.sh"s, "~/start_pykmip.sh");
    test.percona_proxy->copy_to_node(mxt::SOURCE_DIR + "/pinloki/stop_pykmip.sh"s, "~/stop_pykmip.sh");

    int rc = test.percona_proxy->ssh_node_f(false, "./install_pykmip.sh");
    test.expect(rc == 0, "Failed to install PyKMIP");
    rc = test.percona_proxy->ssh_node_f(false, "./start_pykmip.sh");
    test.expect(rc == 0, "Failed to start PyKMIP");

    test.percona_proxy->start();
    auto rv = EncryptionTest(test).result();

    rc = test.percona_proxy->ssh_node_f(false, "./stop_pykmip.sh");
    test.expect(rc == 0, "Failed to stop PyKMIP");
    return rv;
}
