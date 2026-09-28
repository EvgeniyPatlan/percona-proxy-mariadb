# About Percona Proxy for MariaDB

**Percona Proxy for MariaDB** is a database proxy that forwards database statements to
one or more database servers.

The forwarding is performed using rules based on the semantic understanding of
the database statements and on the roles of the servers within the backend
cluster of databases.

Percona Proxy for MariaDB is designed to provide, transparently to applications, load
balancing and high availability functionality. Percona Proxy for MariaDB has a scalable
and flexible architecture, with plugin components to support different protocols
and routing approaches.

Percona Proxy for MariaDB makes extensive use of the asynchronous I/O capabilities of the
Linux operating system, combined with a fixed number of worker threads. *epoll*
is used to provide the event driven framework for the input and output via
sockets.

Many of the services provided by Percona Proxy for MariaDB are implemented as external
shared object modules loaded at runtime. These modules support a fixed
interface, communicating the entry points via a structure consisting of a set of
function pointers. This structure is called the "module object". Additional
modules can be created to work with Percona Proxy for MariaDB.

Commonly used module types are *protocol*, *router* and *filter*. Protocol
modules implement the communication between clients and Percona Proxy for MariaDB, and
between Percona Proxy for MariaDB and backend servers. Routers inspect the queries from
clients and decide the target backend. The decisions are usually based on
routing rules and backend server status. Filters work on data as it passes
through Percona Proxy for MariaDB. Filter are often used for logging queries or modifying
server responses.

A Google Group exists for Percona Proxy for MariaDB. The Group is used to discuss ideas,
issues and communicate with the Percona Proxy for MariaDB community. Send email to
[percona-proxy@googlegroups.com](mailto:percona-proxy@googlegroups.com) or use the
[forum](http://groups.google.com/forum/#!forum/percona-proxy) interface.

Bugs can be reported in the MariaDB Jira
[https://jira.mariadb.org](https://mariadb.atlassian.net)

## Installing Percona Proxy for MariaDB

Information about installing Percona Proxy for MariaDB, either from a repository or by
building from source code, is included in the [Percona Proxy for MariaDB Installation
Guide](../Getting-Started/MariaDB-Percona-Proxy-Installation-Guide.md).

The same guide also provides basic information on running Percona Proxy for MariaDB. More
detailed information about configuring Percona Proxy for MariaDB can be found in the
[Configuration Guide](../Getting-Started/Configuration-Guide.md).
