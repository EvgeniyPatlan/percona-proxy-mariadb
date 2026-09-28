var child_process = require("child_process");
const mariadb = require("mariadb");
var conn = null;
const { spawnSync } = require("node:child_process");
var connectionError = false;

if (process.env.PERCONA_PROXY_DIR == null) {
  throw new Error("PERCONA_PROXY_DIR is not set");
}

const axios = require("axios");
const chai = require("chai");
const assert = require("assert");
const chaiAsPromised = require("chai-as-promised");
chai.use(chaiAsPromised);
const should = chai.should();
const expect = chai.expect;
const host = "http://127.0.0.1:8989/v1/";

const primary_host = "127.0.0.1:8989";
let secondary_host = "127.0.0.1:8990";

if (process.env.percona_proxy2_API) {
  secondary_host = process.env.percona_proxy2_API;
}

function runScript(script) {
  return new Promise(function (resolve, reject) {
    child_process.execFile(script, function (err) {
      if (err) {
        reject(err);
      } else {
        resolve();
      }
    });
  });
}

// Start Percona Proxy, this should be called in the `before` handler of each test unit
function startPerconaProxy() {
  return runScript("./start_percona_proxy.sh");
}

// Stop Percona Proxy, this should be called in the `after` handler of each test unit
function stopPerconaProxy() {
  return runScript("./stop_percona_proxy.sh");
}

// Execute a single Percona Proxyctl command, returns a Promise
function doCommand(command) {
  var percona_proxyctl_cmd = process.env.PERCONA_PROXYCTL_CMD;
  if (percona_proxyctl_cmd == null) {
    // Run the tests directly from the sources
    var ctrl = require("./lib/core.js");
    process.env["PERCONA_PROXYCTL_WARNINGS"] = "0";
    return ctrl.execute(command.split(" "));
  }

  return new Promise(function (resolve, reject) {
    var args = (percona_proxyctl_cmd + " " + command).split(" ");
    const cmd = args.shift();

    var ret = spawnSync(cmd, args, {
      env: { PERCONA_PROXYCTL_WARNINGS: "0" },
    });

    if (ret.status != 0) {
      reject(String(ret.stdout) + String(ret.stdout));
    } else {
      resolve(String(ret.stdout));
    }
  });
}

// Execute a single Percona Proxyctl command and request a resource via the REST API,
// returns a Promise with the JSON format resource as an argument
async function verifyCommand(command, resource) {
  await doCommand(command);
  var res = await axios({
    url: host + resource,
    auth: { username: "admin", password: "mariadb" },
  });
  return res.data;
}

function sleepFor(time) {
  return new Promise((resolve) => {
    setInterval(() => {
      resolve();
    }, time);
  });
}

function isConnectionOk() {
  return connectionError;
}

function createConnection() {
  connectionError = false;
  return mariadb
    .createConnection({ host: "127.0.0.1", port: 4006, user: "maxuser", password: "maxpwd" })
    .then((c) => {
      conn = c;
      conn.on("error", () => {
        connectionError = true;
      });
    })
    .catch(() => {
      connectionError = true;
    });
}

function closeConnection() {
  conn.end();
  conn = null;
}

function getConnectionId() {
  return conn.threadId;
}

async function doQuery(sql, opts) {
  return conn.query(sql, opts ? opts : {});
}

module.exports = {
  axios,
  chai,
  assert,
  should,
  expect,
  host,
  primary_host,
  secondary_host,
  startPerconaProxy,
  stopPerconaProxy,
  doCommand,
  verifyCommand,
  sleepFor,
  isConnectionOk,
  createConnection,
  closeConnection,
  getConnectionId,
  doQuery,
};
