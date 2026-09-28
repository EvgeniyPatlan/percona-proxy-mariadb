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
 * @file bug730.cpp regression case for bug 730 ("Regex filter and shorter than original replacement queries
 * Percona Proxy")
 *
 * - setup regex filter, add it to all percona_proxies->routers[0]
 * @verbatim
 *  [MySetOptionFilter]
 *  type=filter
 *  module=regexfilter
 *  options=ignorecase
 *  match=SET OPTION SQL_QUOTE_SHOW_CREATE
 *  replace=SET SQL_QUOTE_SHOW_CREATE
 *
 *  @endverbatim
 * - try SET OPTION SQL_QUOTE_SHOW_CREATE = 1; against all percona_proxies->routers[0]
 * - check if Maxscale alive
 */

/*
 *  Markus Mäkelä 2015-02-16 10:25:50 UTC
 *  Using the following regex filter:
 *
 *  [MySetOptionFilter]
 *  type=filter
 *  module=regexfilter
 *  options=ignorecase
 *  match=SET OPTION SQL_QUOTE_SHOW_CREATE
 *  replace=SET SQL_QUOTE_SHOW_CREATE
 *
 *  Sending the following query hangs Percona Proxy:
 *
 *  SET OPTION SQL_QUOTE_SHOW_CREATE = 1;
 *
 *  This happens because modutil_replace_SQL doesn't modify the SQL packet length if the resulting replacement
 * is shorter.
 *  Comment 1 Markus Mäkelä 2015-02-16 10:27:20 UTC
 *  Added SQL packet length modifications to modutil_replace_SQL when the original length is different from
 * the replacement length.
 */



#include <iostream>
#include <maxtest/testconnections.hh>

int main(int argc, char* argv[])
{
    TestConnections* Test = new TestConnections(argc, argv);
    Test->reset_timeout();

    Test->percona_proxy->connect_percona_proxy();

    Test->tprintf("RWSplit: \n");
    fflush(stdout);
    Test->try_query(Test->percona_proxy->conn_rwsplit, (char*) "SET OPTION SQL_QUOTE_SHOW_CREATE = 1;");
    Test->tprintf("ReadConn master: \n");
    fflush(stdout);
    Test->try_query(Test->percona_proxy->conn_master, (char*) "SET OPTION SQL_QUOTE_SHOW_CREATE = 1;");
    Test->tprintf("readConn slave: \n");
    fflush(stdout);
    Test->try_query(Test->percona_proxy->conn_slave, (char*) "SET OPTION SQL_QUOTE_SHOW_CREATE = 1;");

    Test->percona_proxy->close_percona_proxy_connections();

    Test->check_percona_proxy_alive();

    int rval = Test->global_result;
    delete Test;
    return rval;
}
