package percona-proxy.java.batch;

import percona-proxy.java.PerconaProxyConfiguration;
import percona-proxy.java.PerconaProxyConnection;
import java.sql.Connection;
import java.sql.Statement;

public class BatchInsert {

    public static void main(String[] args) {
        boolean error = false;
        try {
            PerconaProxyConfiguration config = new PerconaProxyConfiguration("batchinsert");
            PerconaProxyConnection percona-proxy = new PerconaProxyConnection("useBatchMultiSendNumber=500");

            try {
                Connection connection = percona-proxy.getConnRw();
                Statement stmt = connection.createStatement();

                stmt.execute("DROP TABLE IF EXISTS tt");
                stmt.execute("CREATE TABLE tt (d int)");

                for (int i = 0; i < 150; i++) {
                    stmt.addBatch("INSERT INTO tt(d) VALUES (1)");

                    if (i % 3 == 0) {
                        stmt.addBatch("SET @test2='aaa'");
                    }
                }

                stmt.executeBatch();
                System.out.println("finished");

            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
                error = true;
            }
            config.close();

        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }

        if (error) {
            System.exit(1);
        }
    }
}
