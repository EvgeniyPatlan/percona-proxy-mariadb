#include <maxtest/testconnections.hh>
#include <sstream>

// Base class for Pinloki test cases. Provides some utility functions related to replication as well as common
// testing functionality.
enum class GtidPos
{
    SLAVE,
    CURRENT
};

std::string change_master_sql(const char* host, int port,
                              GtidPos type = GtidPos::SLAVE,
                              const char* user = "maxskysql",
                              const char* password = "skysql")
{
    std::ostringstream ss;

    ss << "CHANGE MASTER TO MASTER_HOST='" << host << "', MASTER_PORT=" << port
       << ", MASTER_USER='" << user << "', MASTER_PASSWORD='" << password
       << "', MASTER_USE_GTID=" << (type == GtidPos::SLAVE ? "SLAVE_POS" : "CURRENT_POS")
       << ", MASTER_SSL_VERIFY_SERVER_CERT=0";

    return ss.str();
}

class TestCase
{
public:
    TestCase(TestConnections& t)
        : test(t)
        , master(test.repl->get_connection(0))
        , slave(test.repl->get_connection(1))
        , percona_proxy(test.percona_proxy->rwsplit())
    {
    }

    // Runs the test and returns the result code (0 for no errors)
    int result()
    {
        setup();
        pre();

        if (test.ok())
        {
            run();
            post();
        }

        return test.global_result;
    }

protected:
    //
    // The following should be overridden by the test case
    //

    // The actual test
    virtual void run() = 0;

    // Any steps needed to be done before the test
    virtual void pre()
    {
    }

    // Any cleanup that needs to be done that was done in `pre`
    virtual void post()
    {
    }

    // Test setup. Connects all Connections and sets up replication between
    // the master, Percona Proxy and a slave. Only override if custom test setup is needed.
    virtual void setup()
    {
        test.expect(percona_proxy.connect(), "Pinloki connection should work: %s", percona_proxy.error());
        test.expect(master.connect(), "Master connection should work: %s", master.error());
        test.expect(slave.connect(), "Slave connection should work: %s", slave.error());

        // Use the latest GTID in case the binlogs have been purged and the complete history is not available
        auto gtid = master.field("SELECT @@gtid_current_pos");

        // Stop the slave while we configure pinloki
        slave.query("STOP SLAVE; RESET SLAVE ALL;");

        // Start replicating from the master
        percona_proxy.query("STOP SLAVE");
        percona_proxy.query("RESET SLAVE");
        percona_proxy.query("SET GLOBAL gtid_slave_pos = '" + gtid + "'");
        percona_proxy.query(change_master_sql(test.repl->ip(0), test.repl->port(0)));
        percona_proxy.query("START SLAVE");

        // Sync Percona Proxy with the master
        sync(master, percona_proxy);

        // Configure the slave to replicate from Percona Proxy and sync it
        slave.query("SET GLOBAL gtid_slave_pos = '" + gtid + "'");
        slave.query(change_master_sql(test.percona_proxy->ip(), test.percona_proxy->rwsplit_port));
        slave.query("START SLAVE");
        sync(percona_proxy, slave);
    }

    virtual void setup_select_master()
    {
        test.expect(percona_proxy.connect(), "Pinloki connection should work: %s", percona_proxy.error());
        test.expect(master.connect(), "Master connection should work: %s", master.error());
        test.expect(slave.connect(), "Slave connection should work: %s", slave.error());

        // Use the latest GTID in case the binlogs have been purged and the complete history is not available
        auto gtid = master.field("SELECT @@gtid_current_pos");

        percona_proxy.query("STOP SLAVE");
        percona_proxy.query("SET GLOBAL gtid_slave_pos = '" + gtid + "'");
        percona_proxy.query("START SLAVE");

        sync(master, percona_proxy);

        slave.query("STOP SLAVE; RESET SLAVE ALL;");
        slave.query(change_master_sql(test.percona_proxy->ip(), test.percona_proxy->rwsplit_port));
        slave.query("START SLAVE");
        sync(percona_proxy, slave);
    }

protected:
    TestConnections& test;      // The core test library
    Connection       master;    // Connection to the master
    Connection       slave;     // Connection to the slave
    Connection       percona_proxy;  // Connection to Percona Proxy

    // Syncs the `dest` connection with the `src` connection
    void sync(Connection& src, Connection& dest)
    {
        auto gtid = src.field("SELECT @@gtid_current_pos");
        auto start_gtid = dest.field("SELECT @@gtid_current_pos");
        auto res = dest.field("SELECT MASTER_GTID_WAIT('" + gtid + "', 30)");
        test.expect(res == "0",
                    "`MASTER_GTID_WAIT('%s', 30)` on %s returned: %s (error: %s). "
                    "Target GTID: %s Starting GTID: %s",
                    gtid.c_str(), &dest == &percona_proxy ? "Percona Proxy" : "slave",
                    res.c_str(), slave.error(), gtid.c_str(), start_gtid.c_str());
    }

    // Syncs percona-proxy with master and then the slave with master
    void sync_all()
    {
        sync(master, percona_proxy);
        sync(percona_proxy, slave);
    }

    // Checks that `master`, `percona-proxy` and `slave` all report the same GTID position
    void check_gtid()
    {
        auto master_pos = master.field("SELECT @@gtid_current_pos");
        auto slave_pos = slave.field("SELECT @@gtid_current_pos");
        auto percona_proxy_pos = percona_proxy.field("SELECT @@gtid_current_pos");

        test.expect(percona_proxy_pos == master_pos,
                    "Percona Proxy GTID (%s) is not the same as Master GTID (%s)",
                    percona_proxy_pos.c_str(), master_pos.c_str());

        test.expect(slave_pos == percona_proxy_pos,
                    "Slave GTID (%s) is not the same as Percona Proxy GTID (%s)",
                    slave_pos.c_str(), percona_proxy_pos.c_str());
    }
};
