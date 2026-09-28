// These tests use the server/test/percona-proxy-test.cnf configuration

require("../utils.js")();
const mariadb = require("mariadb");
var conn;

function createConnection() {
  return mariadb
    .createConnection({ host: "127.0.0.1", port: 4006, user: "maxuser", password: "maxpwd" })
    .then((c) => {
      conn = c;
    });
}

function closeConnection() {
  conn.end();
  conn = null;
}

describe("Schema Validation", function () {
  before(createConnection);

  describe("Resource Collections", function () {
    var tests = [
      "/servers",
      "/sessions",
      "/services",
      "/monitors",
      "/filters",
      "/listeners",
      "/percona-proxy/threads",
      "/percona-proxy/modules",
      "/users",
      "/users/inet",
      "/users/unix",
    ];

    tests.forEach(function (endpoint) {
      it(endpoint + ": resource found", function () {
        return request.get(base_url + endpoint).should.be.fulfilled;
      });

      it(endpoint + ": resource schema is valid", function () {
        return request.get(base_url + endpoint).should.eventually.satisfy(validate);
      });
    });
  });

  describe("Individual Resources", function () {
    var tests = [
      "/servers/server1",
      "/servers/server2",
      "/services/RW-Split-Router",
      "/services/RW-Split-Router/listeners",
      "/listeners/RW-Split-Listener",
      "/monitors/MariaDB-Monitor",
      "/filters/Hint",
      "/sessions/1",
      "/percona-proxy/",
      "/percona-proxy/query_classifier/cache",
      "/percona-proxy/threads/0",
      "/percona-proxy/logs",
      "/percona-proxy/memory",
      "/percona-proxy/modules/readwritesplit",
    ];

    tests.forEach(function (endpoint) {
      it(endpoint + ": resource found", function () {
        return request.get(base_url + endpoint).should.be.fulfilled;
      });

      it(endpoint + ": resource schema is valid", function () {
        return request.get(base_url + endpoint).should.eventually.satisfy(validate);
      });
    });
  });

  describe("Resource Self Links", function () {
    var tests = [
      "/servers",
      "/sessions",
      "/services",
      "/monitors",
      "/filters",
      "/listeners",
      "/percona-proxy/query_classifier/cache",
      "/percona-proxy/threads",
      "/percona-proxy/modules",
      "/servers/server1",
      "/servers/server2",
      "/services/RW-Split-Router",
      "/services/RW-Split-Router/listeners",
      "/services/RW-Split-Router/listeners/RW-Split-Listener",
      "/listeners/RW-Split-Listener",
      "/monitors/MariaDB-Monitor",
      "/filters/Hint",
      "/sessions/1",
      "/percona-proxy/",
      "/percona-proxy/threads/0",
      "/percona-proxy/logs",
      "/percona-proxy/modules/readwritesplit",
    ];

    tests.forEach(function (endpoint) {
      it(endpoint + ": correct self link", async function () {
        var obj = await request.get(base_url + endpoint);
        var obj_self = await request.get(obj.links.self);
        obj_self.links.self.should.be.equal(obj.links.self);
      });
    });
  });

  describe("Resource Relationship Self Links", function () {
    const endpoints = {
      servers: ["services", "monitors"],
      services: ["servers", "services", "filters", "monitors"],
      monitors: ["servers", "services"],
      filters: ["services"],
      listeners: ["services"],
      sessions: ["services"],
    };

    for (k of Object.keys(endpoints)) {
      it(k + ": correct resource self link", async function () {
        var res = await request.get(base_url + "/" + endpoints[k]);

        for (o of res.data) {
          for (r of endpoints[k]) {
            if (o.relationships[r]) {
              var self = await request.get(o.relationships[r].links.self);
              self.should.be.deep.equal(o.relationships[r]);
            }
          }
        }
      });
    }
  });

  after(closeConnection);
});
