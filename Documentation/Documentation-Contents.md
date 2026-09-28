
# Contents

## About Percona Proxy for MariaDB

 - [About Percona Proxy for MariaDB](About/About-Percona-Proxy.md)
 - [Changelog](Changelog.md)
 - [Limitations](About/Limitations.md)

## Getting Started

 - [Percona Proxy for MariaDB Installation Guide](Getting-Started/MariaDB-Percona-Proxy-Installation-Guide.md)
 - [Building Percona Proxy for MariaDB from Source Code](Getting-Started/Building-Percona-Proxy-from-Source-Code.md)
 - [Configuration Guide](Getting-Started/Configuration-Guide.md)
 - [Percona Proxy GUI](Getting-Started/Percona-Proxy GUI.md)

## Upgrading Percona Proxy for MariaDB

- [Upgrading Percona Proxy](Upgrading/Upgrading-Percona-Proxy.md)

## Reference

 - [Percona Proxyctl - Command Line Admin Interface](Reference/Percona-Proxyctl.md)
 - [Percona Proxy REST API](REST-API/API.md)
 - [Module Commands](Reference/Module-Commands.md)
 - [Routing Hints](Reference/Hint-Syntax.md)

## Tutorials

The main tutorial for Percona Proxy for MariaDB consist of setting up Percona Proxy for MariaDB for the environment you are using with either a connection-based or a read/write-based configuration.

 - [Percona Proxy for MariaDB Tutorial](Tutorials/Percona-Proxy-Tutorial.md)

These tutorials are for specific use cases and module combinations.

 - [Administration Tutorial](Tutorials/Administration-Tutorial.md)
 - [Avro Router Tutorial](Tutorials/Avrorouter-Tutorial.md)
 - [Connection Routing Tutorial](Tutorials/Connection-Routing-Tutorial.md)
 - [Filter Tutorial](Tutorials/Filter-Tutorial.md)
 - [MariaDB Monitor Failover Tutorial](Tutorials/MariaDB-Monitor-Failover.md)
 - [Read Write Splitting Tutorial](Tutorials/Read-Write-Splitting-Tutorial.md)
 - [Simple Schema Sharding Tutorial](Tutorials/Simple-Sharding-Tutorial.md)

Here are tutorials on monitoring and managing Percona Proxy for MariaDB in cluster environments.

 - [REST API Tutorial](Tutorials/REST-API-Tutorial.md)

## Routers

The routing module is the core of a Percona Proxy for MariaDB service. The router documentation
contains all module specific configuration options and detailed explanations
of their use.

 - [Avrorouter](Routers/Avrorouter.md)
 - [Binlogrouter](Routers/Binlogrouter.md)
 - [Cat](Routers/Cat.md)
 - [KafkaCDC](Routers/KafkaCDC.md)
 - [KafkaImporter](Routers/KafkaImporter.md)
 - [MirrorRouter](Routers/Mirror.md)
 - [Read Connection Router](Routers/ReadConnRoute.md)
 - [Read Write Split](Routers/ReadWriteSplit.md)
 - [Schemarouter](Routers/SchemaRouter.md)
 - [SmartRouter](Routers/SmartRouter.md)

## Filters

Here are detailed documents about the filters Percona Proxy for MariaDB offers. They contain configuration guides and example use cases. Before reading these, you should have read the filter tutorial so that you know how they work and how to configure them.

 - [Binlog Filter](Filters/BinlogFilter.md)
 - [Cache](Filters/Cache.md)
 - [Comment Filter](Filters/Comment.md)
 - [Consistent Critical Read Filter](Filters/CCRFilter.md)
 - [Hint Filter](Filters/Hintfilter.md)
 - [LDIFilter](Filters/LDIFilter.md)
 - [Masking Filter](Filters/Masking.md)
 - [Maxrows Filter](Filters/Maxrows.md)
 - [Named Server Filter](Filters/Named-Server-Filter.md)
 - [Query Log All](Filters/Query-Log-All-Filter.md)
 - [Regex Filter](Filters/Regex-Filter.md)
 - [Rewrite Filter](Filters/RewriteFilter.md)
 - [Tee Filter](Filters/Tee-Filter.md)
 - [Throttle Filter](Filters/Throttle.md)
 - [Top N Filter](Filters/Top-N-Filter.md)

## Monitors

Common options for all monitor modules.

 - [Monitor Common](Monitors/Monitor-Common.md)

Module specific documentation.

 - [Galera Monitor](Monitors/Galera-Monitor.md)
 - [MariaDB Monitor](Monitors/MariaDB-Monitor.md)

## Protocols

Documentation for Percona Proxy protocol modules.

 - [MariaDB](Protocols/MariaDB.md)
 - [Change Data Capture (CDC) Protocol](Protocols/CDC.md)
 - [Change Data Capture (CDC) Users](Protocols/CDC_users.md)
 - [NoSQL](Protocols/NoSQL.md)

The Percona Proxy CDC Connector provides a C++ API for consuming data from a CDC system.

 - [CDC Connector](Connectors/CDC-Connector.md)

## Authenticators

A short description of the authentication module type can be found in the
[Authentication Modules](Authenticators/Authentication-Modules.md)
document.

 - [MariaDB/MySQL Authenticator](Authenticators/MySQL-Authenticator.md)
 - [GSSAPI Authenticator](Authenticators/GSSAPI-Authenticator.md)
 - [PAM Authenticator](Authenticators/PAM-Authenticator.md)
 - [Ed25519 Authenticator](Authenticators/Ed25519-Authenticator.md)
