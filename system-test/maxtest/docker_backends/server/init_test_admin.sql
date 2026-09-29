-- The test framework opens an admin connection as this user and creates the rest of the test
-- users through it (see admin_user in maxtest/src/mariadb_nodes.cc). When the backends are
-- virtual machines, MDBCI creates the account while provisioning them; an image has to create
-- it itself, or every test fails with "Access denied for user 'test-admin'".
--
-- This runs only when the data directory is initialised, which is what happens when a test
-- recreates a container together with a fresh volume.
CREATE USER IF NOT EXISTS 'test-admin'@'%' IDENTIFIED BY 'test-admin-pw';
GRANT ALL ON *.* TO 'test-admin'@'%' WITH GRANT OPTION;
FLUSH PRIVILEGES;
