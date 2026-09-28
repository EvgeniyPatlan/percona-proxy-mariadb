#include <maxtest/testconnections.hh>
#include "test_base.hh"

std::string case1 = "Earth is the third planet";
std::string case2 = "Mars is the fourth planet";
std::string case3 = "Jupiter is the fifth planet";
std::string case4 = "Saturn is the sixth planet";
std::string case5 = "Uranus is the seventh planet";

class EncryptionTest : public TestCase
{
public:
    using TestCase::TestCase;

    void query(std::string query)
    {
        test.expect(master.query(query), "Query '%s' failed: %s", query.c_str(), master.error());
    }

    void check_contents(std::string tbl, std::string str)
    {
        auto result = slave.field("SELECT a FROM " + tbl + " WHERE a = '" + str + "'");
        test.expect(result == str,
                    "%s should have a row with '%s' in it.",
                    tbl.c_str(), str.c_str());
    }

    void check_encryption(std::initializer_list<std::string> strs)
    {
        auto rv = test.percona_proxy->ssh_output("find /var/lib/percona-proxy/binlogs/ -type f -exec strings {} \\;");

        for (const auto& str : strs)
        {
            test.expect(rv.output.find(str) == std::string::npos,
                        "'%s' should not be visible in the binlogs: %s",
                        str.c_str(), rv.output.c_str());
        }
    }

    void setup() override
    {
        setup_select_master();
    }

    void pre() override
    {
        query("CREATE TABLE test.t1(a VARCHAR(255))");
        query("INSERT INTO test.t1 VALUES ('" + case1 + "')");
        query("FLUSH LOGS");
        query("CREATE TABLE test.t2 (a TEXT)");
        query("INSERT INTO test.t2 VALUES ('" + case1 + "')");
        sync_all();
    }

    void run() override
    {
        test.tprintf("Sanity check");
        check_contents("test.t1", case1);
        check_contents("test.t2", case1);
        check_gtid();

        // Restart Percona Proxy and insert new values. Old values should not be visible.
        test.percona_proxy->restart();

        query("INSERT INTO test.t1 VALUES ('" + case2 + "')");
        query("INSERT INTO test.t2 VALUES ('" + case3 + "')");

        test.tprintf("Encryption after restart");
        // Reconnect to Percona Proxy since it was restarted and force the slave to reconnect as well.
        percona_proxy.connect();
        slave.query("STOP SLAVE; START SLAVE;");
        sync_all();

        check_contents("test.t1", case2);
        check_contents("test.t2", case3);

        check_encryption({case1, case2, case3, case4, case5});

        test.tprintf("Encrypted binlogs with bad configuration should not work");
        test.percona_proxy->ssh_node_f(true, "sed -i 's/encryption_key_id/#encryption_key_id/' /etc/percona-proxy.cnf");
        test.percona_proxy->restart();
        test.expect(percona_proxy.connect(), "Connection should work: %s", percona_proxy.error());
        slave.query("STOP SLAVE; START SLAVE;");

        query("FLUSH LOGS");
        query("INSERT INTO test.t1 VALUES ('unencrypted1')");
        query("INSERT INTO test.t2 VALUES ('unencrypted2')");

        test.percona_proxy->ssh_node_f(true, "sed -i 's/#encryption_key_id/encryption_key_id/' /etc/percona-proxy.cnf");
        test.percona_proxy->restart();
        test.expect(percona_proxy.connect(), "Connection should work: %s", percona_proxy.error());
        slave.query("STOP SLAVE; START SLAVE;");

        query("FLUSH LOGS");
        query("INSERT INTO test.t1 VALUES ('unencrypted3')");
        query("INSERT INTO test.t2 VALUES ('unencrypted4')");
        sync_all();
    }

    void post() override
    {
        test.expect(master.query("DROP TABLE test.t1"), "DROP failed: %s", master.error());
        test.expect(master.query("DROP TABLE test.t2"), "DROP failed: %s", master.error());
    }
};
