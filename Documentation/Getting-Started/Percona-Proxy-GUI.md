# Percona Proxy for MariaDB Percona Proxy GUI Guide

[TOC]

# Introduction

_Percona Proxy GUI_ is a browser-based interface for Percona Proxy REST-API and query execution.

# Enabling Percona Proxy GUI

To enable Percona Proxy GUI in a testing mode, add `admin_host=0.0.0.0` and
`admin_secure_gui=false` under the `[percona-proxy]` section of the Percona Proxy
configuration file. Once enabled, Percona Proxy GUI will be available on port 8989:
`http://127.0.0.1:8989/`

## Securing the GUI

To make Percona Proxy GUI secure, set `admin_secure_gui=true` and configure both the
`admin_ssl_key` and `admin_ssl_cert` parameters.

See [Configuration Guide](./Configuration-Guide.md) and
[Configuration and Hardening](../Tutorials/REST-API-Tutorial.md#configuration-and-hardening)
for instructions on how to harden your Percona Proxy installation for production use.

# Authentication

Percona Proxy GUI uses the same credentials as `percona-proxyctl`. The default username is `admin`
with `mariadb` as the password.

Internally, Percona Proxy GUI uses [JSON Web Tokens](https://jwt.io/introduction/) as the
authentication method for persisting the user's session. If the _Remember me_
checkbox is ticked, the session will persist for 24 hours. Otherwise, the
session will expire as soon as Percona Proxy GUI is closed.

To log out, simply click the username section in the top right corner of the
page header to access the logout menu.

# Pages

## Dashboard

This page provides an overview of Percona Proxy configuration which includes
Monitors, Servers, Services, Sessions, Listeners, and Filters.

By default, the refresh interval is 10 seconds.

## Detail

This page shows information on each
[Percona Proxy object](./Configuration-Guide.md#objects) and allow to edit its
parameter, relationships and perform other manipulation operations.

Access this page by clicking on the Percona Proxy object name on the
[dashboard page](#dashboard)

## Visualization

This page visualizes Percona Proxy configuration and clusters.

- Configuration: Visualizing Percona Proxy configuration.
- Cluster: Visualizing a replication cluster into a tree graph and provides
  manual cluster manipulation operations such as
  `switchover, reset-replication, release-locks, failover, rejoin` . At the
  moment, it supports only servers monitored by Monitor using
  [mariadbmon](../Monitors/MariaDB-Monitor.md) module.

Access this page by clicking the graph icon on the sidebar navigation.

## Settings

This page shows and allows editing of Percona Proxy parameters.

Access this page by clicking the gear icon on the sidebar navigation.

## Logs Archive

Realtime Percona Proxy logs can be accessed by clicking the logs icon on the sidebar
navigation.

## Workspace

The "Workspace" page offers a versatile set of tools for effectively managing
data and database interactions. It includes the following key tasks:

### 1. Run Queries

Execute queries on various servers, services, or listeners to retrieve data and
perform database operations. Visualize query results using different graph
types such as line, bar, or scatter graphs. Export query results in formats
like CSV or JSON for further analysis and sharing.

### 2. Data Migration

The "Data Migration" feature facilitates seamless transitions from PostgreSQL
to MariaDB. Transfer data and database structures between the two systems while
ensuring data integrity and consistency throughout the process.

### 3. Create an ERD

Generating Entity-Relationship Diagrams (ERDs) to gain insights regarding data
structure, optimizing database design for both efficiency and clarity.
