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
#include <maxtest/testconnections.hh>

using namespace std;

namespace
{

void init(TestConnections& test)
{
    MYSQL* pMysql = test.percona_proxy->conn_rwsplit;

    test.try_query(pMysql, "DROP TABLE IF EXISTS MXS_1719");
    test.try_query(pMysql, "CREATE TABLE MXS_1719 (a TEXT, b TEXT)");
    test.try_query(pMysql, "INSERT INTO MXS_1719 VALUES (1, 1)");
}

void run(TestConnections& test)
{
    init(test);

    MYSQL* pMysql = mysql_init(NULL);
    test.expect(pMysql, "Could not create MYSQL handle.");

    const char* zUser = test.percona_proxy->user_name().c_str();
    const char* zPassword = test.percona_proxy->password().c_str();
    int port = test.percona_proxy->rwsplit_port;

    if (mysql_real_connect(pMysql,
                           test.percona_proxy->ip4(),
                           zUser,
                           zPassword,
                           "test",
                           port,
                           NULL,
                           CLIENT_MULTI_STATEMENTS))
    {
        const char* q = "UPDATE MXS_1719 SET a=1; UPDATE MXS_1719 SET a=1;";
        // One multi-statement with two UPDATEs. Note: This query should fail
        // with 2.3 now that function blocking has been added
        test.expect(execute_query_silent(pMysql, q) != 0, "Query '%s' should not succeed", q);

        // Sleep a while, so that the log is flushed.
        sleep(5);
        // This is actually related to MXS-1861 "masking filter logs warnings with
        // multistatements" but it seems excessive to create a specific test for that.
        test.log_excludes("Received data, although expected nothing");

        // This will hang immediately, so we can shorten the timeout.
        test.reset_timeout();
        test.try_query(pMysql, "SELECT * FROM MXS_1719");
    }
    else
    {
        test.expect(false, "Could not connect to Percona Proxy.");
    }

    mysql_close(pMysql);
}
}

int main(int argc, char* argv[])
{
    TestConnections::skip_percona_proxy_start(true);

    TestConnections test(argc, argv);
    std::string src = mxt::SOURCE_DIR;
    src += "/mxs1719.json";
    std::string dst = std::string(test.percona_proxy->access_homedir()) + "/mxs1719.json";

    if (test.percona_proxy->copy_to_node(src.c_str(), dst.c_str()))
    {
        test.percona_proxy->ssh_node((std::string("chmod a+r ") + dst).c_str(), true);
        test.percona_proxy->start();
        if (test.ok())
        {
            test.percona_proxy->wait_for_monitor();

            if (test.percona_proxy->connect_rwsplit() == 0)
            {
                run(test);
            }
            else
            {
                test.expect(false, "Could not connect to RWS.");
            }
        }
    }
    else
    {
        test.expect(false, "Could not copy masking file to Percona Proxy node.");
    }

    test.percona_proxy->connect();
    test.try_query(test.percona_proxy->conn_rwsplit, "DROP TABLE MXS_1719");
    test.percona_proxy->disconnect();

    return test.global_result;
}
