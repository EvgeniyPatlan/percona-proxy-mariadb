/*
 * Copyright (c) 2016 MariaDB Corporation Ab
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

#include <percona-proxy/percona-proxy.hh>

#include <ctime>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>

#include <maxbase/pretty_print.hh>
#include <percona-proxy/mainworker.hh>
#include <percona-proxy/build_details.hh>
#include <percona-proxy/config.hh>
#include <percona-proxy/utils.hh>
#include <percona-proxy/version.hh>

#include "internal/percona-proxy.hh"

namespace
{
time_t started;
sig_atomic_t n_shutdowns {0};
bool teardown_in_progress {false};
}

void percona_proxy_reset_starttime()
{
    started = time(nullptr);
}

time_t percona_proxy_started()
{
    return started;
}

int percona_proxy_uptime()
{
    return time(nullptr) - started;
}

bool percona_proxy_is_shutting_down()
{
    return n_shutdowns != 0;
}

int percona_proxy_shutdown()
{
    int n = n_shutdowns++;

    if (n == 0)
    {
        mxs::MainWorker::get()->execute_signal_safe(&mxs::MainWorker::start_shutdown);
    }

    return n + 1;
}

bool percona_proxy_teardown_in_progress()
{
    return teardown_in_progress;
}

void percona_proxy_start_teardown()
{
    teardown_in_progress = true;
}

const char* percona_proxy_commit()
{
    return PERCONA_PROXY_COMMIT;
}

const char* percona_proxy_source()
{
    return PERCONA_PROXY_SOURCE;
}

const char* percona_proxy_cmake_flags()
{
    return PERCONA_PROXY_CMAKE_FLAGS;
}

const char* percona_proxy_jenkins_build_tag()
{
    return PERCONA_PROXY_JENKINS_BUILD_TAG;
}

void percona_proxy_log_info_blurb(LogBlurbAction action)
{
    const char* verb = action == LogBlurbAction::STARTUP ? "started " : "";
    struct sysinfo info;
    sysinfo(&info);

    const mxs::Config& cnf = mxs::Config::get();
    MXB_NOTICE("Host: '%s' OS: %s, %s@%s, %s, %s with %ld processor cores (%.2f available).",
               cnf.nodename.c_str(),  cnf.release_string.c_str(), cnf.sysname.c_str(), cnf.release.c_str(),
               cnf.version.c_str(), cnf.machine.c_str(), get_processor_count(),
               get_vcpu_count());

    MXB_NOTICE("Total main memory: %s (%s usable).",
               mxb::pretty_size(get_total_memory()).c_str(),
               mxb::pretty_size(get_available_memory()).c_str());
    MXB_NOTICE("Percona Proxy is running in process %i", getpid());
    MXB_NOTICE("Percona Proxy for MariaDB %s %s(Commit: %s)", PERCONA_PROXY_VERSION, verb, percona_proxy_commit());

    const char* thp_enable_path = "/sys/kernel/mm/transparent_hugepage/enabled";
    std::string line;
    std::getline(std::ifstream(thp_enable_path), line);

    if (line.find("[always]") != std::string::npos)
    {
        MXB_NOTICE("Transparent hugepages are set to 'always', Percona Proxy may end up using more memory "
                   "than it needs. To disable it, set '%s' to 'madvise' ", thp_enable_path);
    }
}
