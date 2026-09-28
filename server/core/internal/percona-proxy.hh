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
 * @file core/percona-proxy/percona-proxy.hh - The private percona-proxy general definitions
 */

#include <percona-proxy/percona-proxy.hh>

/**
 * Initiate shutdown of Percona Proxy.
 *
 * This functions informs all threads that they should stop the
 * processing and exit. This should only be called by the SIGTERM and SIGINT signal handlers.
 *
 * @return How many times percona_proxy_shutdown() has been called.
 */
int percona_proxy_shutdown();

/**
 * Reset the start time from which the uptime is calculated.
 */
void percona_proxy_reset_starttime();

// Helper functions for debug assertions
bool percona_proxy_teardown_in_progress();
void percona_proxy_start_teardown();

enum class LogBlurbAction {STARTUP, LOG_ROTATION};

/**
 * Log the details of the Percona Proxy and the system it is running on
 *
 * Should be called on startup and whenever the log is rotated.
 *
 * @param type The action type
 */
void percona_proxy_log_info_blurb(LogBlurbAction type);
