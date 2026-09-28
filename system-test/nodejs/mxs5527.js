// MXS-5527: INSERT...RETURNING doesn't work with causal reads

const mariadb = require("mariadb");
const assert = require("assert").strict;

async function asyncFunction() {
  const conn = await mariadb.createConnection({
    host: process.env.PERCONA_PROXY_HOST,
    port: process.env.PERCONA_PROXY_PORT,
    user: process.env.PERCONA_PROXY_USER,
    password: process.env.PERCONA_PROXY_PASSWORD,
    database: process.env.PERCONA_PROXY_DB,
  });
  try {
    await conn.query("INSERT INTO t1 VALUES (1) RETURNING id");

    const result = await conn.query(
      "SELECT @@server_id AS id, COUNT(*) AS count FROM t1"
    );
    const expected = await conn.query(
      "SELECT @@server_id AS id, COUNT(*) AS count FROM t1 WHERE @@last_insert_id = @@last_insert_id"
    );

    assert.deepEqual(result, expected);
  } catch (err) {
    throw err;
  } finally {
    if (conn) conn.end();
  }
}

asyncFunction().then(console.log, console.log);
