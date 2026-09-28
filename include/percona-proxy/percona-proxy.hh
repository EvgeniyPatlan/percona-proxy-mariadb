/*
 * Copyright (c) 2018 MariaDB Corporation Ab
 * Copyright (c) 2023 MariaDB plc, Finnish Branch
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2026-09-21
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */
#pragma once

/**
 * @file include/percona-proxy/percona-proxy.h Some general definitions for Percona Proxy
 */

#include <percona-proxy/ccdefs.hh>

#include <ctime>

/**
 * Return the time when Percona Proxy was started.
 */
time_t percona_proxy_started();

/**
 * Return the time Percona Proxy has been running.
 *
 * @return The uptime in seconds.
 */
int percona_proxy_uptime();

/**
 * Is Percona Proxy shutting down
 *
 * This function can be used to detect whether the shutdown has been initiated. It does not tell
 * whether the shutdown has been completed so thread-safety is still important.
 *
 * @return True if Percona Proxy is shutting down
 */
bool percona_proxy_is_shutting_down();

const char* percona_proxy_commit();
const char* percona_proxy_source();
const char* percona_proxy_cmake_flags();
const char* percona_proxy_jenkins_build_tag();
