package percona-proxy.java.test1;

import percona-proxy.java.PerconaProxyConfiguration;
import percona-proxy.java.PerconaProxyConnection;

public class SimpleConnectorJTest {

    public static final int RWSPLIT_PORT = 4006;
    public static final int READCONN_MASTER = 4008;
    public static final int READCONN_SLAVE = 4009;
    public static final String DATABASE_NAME = "mytestdb";
    public static final String TABLE_NAME = "t1";
    public static final int ITERATIONS_NORMAL = 1500;
    public static final int ITERATIONS_SMOKE = 150;
    public static int test_rows = ITERATIONS_NORMAL;

    public static void main(String[] args) {
        boolean error = false;

        try {
            PerconaProxyConfiguration config = new PerconaProxyConfiguration("simplejavatest");
            PerconaProxyConnection percona-proxy = new PerconaProxyConnection();
            try {

                if (percona-proxy.isSmokeTest()) {
                    test_rows = ITERATIONS_SMOKE;
                }

                System.out.println("Creating databases and tables..");
                percona-proxy.query(percona-proxy.getConnMaster(), "DROP DATABASE IF EXISTS " + DATABASE_NAME);
                percona-proxy.query(percona-proxy.getConnMaster(), "CREATE DATABASE " + DATABASE_NAME);
                percona-proxy.query(percona-proxy.getConnMaster(), "CREATE TABLE " + DATABASE_NAME
                               + "." + TABLE_NAME + "(id int primary key auto_increment, data varchar(128))");

                System.out.println("Inserting " + test_rows + " values");
                for (int i = 0; i < test_rows; i++) {
                    percona-proxy.query(percona-proxy.getConnMaster(),
                                   "INSERT INTO " + DATABASE_NAME + "." + TABLE_NAME
                                   + "(data) VALUES (" + String.valueOf(System.currentTimeMillis()) + ")");
                }

                System.out.println("Querying " + test_rows / 10 + "rows " + test_rows + " times");
                for (int i = 0; i < test_rows; i++) {
                    percona-proxy.query(percona-proxy.getConnMaster(),
                                   "SELECT * FROM " + DATABASE_NAME + "." + TABLE_NAME
                                   + " LIMIT " + test_rows / 10);
                }

                percona-proxy.query(percona-proxy.getConnMaster(), "DROP DATABASE IF EXISTS " + DATABASE_NAME);

            } catch (Exception ex) {
                error = true;
                System.out.println("Error: " + ex.getMessage());
                ex.printStackTrace();
            }

            config.close();

        } catch (Exception ex) {
            error = true;
            System.out.println("Error: " + ex.getMessage());
            ex.printStackTrace();
        }

        if (error) {
            System.exit(1);
        }
    }
}
