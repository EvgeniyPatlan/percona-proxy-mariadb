#include <maxtest/testconnections.hh>
#include "test_base.hh"

class MasterSelectTest : public TestCase
{
public:
    using TestCase::TestCase;

    void setup() override
    {
        setup_select_master();
    }

    void run() override
    {
        test.expect(!percona_proxy.query(change_master_sql(test.repl->ip(0), test.repl->port(0))),
                    "CHANGE MASTER should fail");
        test.expect(percona_proxy.query("STOP SLAVE"), "STOP SLAVE should work: %s", percona_proxy.error());
        test.expect(percona_proxy.query("START SLAVE"), "START SLAVE should work: %s", percona_proxy.error());

        check_gtid();

        test.expect(master.query("CREATE TABLE test.t1(id INT)"), "CREATE failed: %s", master.error());
        test.expect(master.query("INSERT INTO test.t1 VALUES (1)"), "INSERT failed: %s", master.error());
        test.expect(master.query("DROP TABLE test.t1"), "DROP failed: %s", master.error());

        sync(master, percona_proxy);
        sync(percona_proxy, slave);

        check_gtid();
    }
};
