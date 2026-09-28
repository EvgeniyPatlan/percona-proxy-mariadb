/*
 * Copyright (c) 2026 Percona LLC and/or its affiliates
 *
 * Use of this software is governed by version 2 or later of the General Public License.
 */

/**
 * Percona Proxy for MariaDB is derived from MariaDB MaxScale, where the global configuration
 * section was called [maxscale]. A configuration that has not been migrated is still read, with
 * the section treated as if it were named [percona-proxy], so that the global settings of an
 * unmigrated installation are not silently ignored.
 */

#include <iostream>
#include <maxbase/log.hh>
#include <percona-proxy/cn_strings.hh>
#include <percona-proxy/percona_proxy_test.h>
#include "../internal/config.hh"

using namespace std;

namespace
{

struct TestCase
{
    const char* zName;
    const char* zConfig;
    bool        expect_global_section;    // a section named CN_MAXSCALE is present afterwards
    bool        expect_warning;
    bool        expect_error;
} test_cases[] =
{
    {
        "The current name is accepted without a warning",
        R"([percona-proxy]
threads=4
)",
        true, false, false
    },
    {
        "The MaxScale name is accepted, renamed, and warned about",
        R"([maxscale]
threads=4
)",
        true, true, false
    },
    {
        "The MaxScale name in a different case is accepted too",
        R"([MaxScale]
threads=4
)",
        true, true, false
    },
    {
        "A case variant of the current name keeps working",
        R"([Percona-Proxy]
threads=4
)",
        true, true, false
    },
    {
        // Reported as a duplicate, exactly as two sections differing only in case are. The
        // sections are left as they are, because the error rejects the configuration anyway.
        "Both names at once is a duplicate",
        R"([maxscale]
threads=4

[percona-proxy]
threads=8
)",
        true, false, true
    },
    {
        "An unrelated section is left alone",
        R"([server1]
type=server
address=127.0.0.1
)",
        false, false, false
    },
};

int test(const TestCase& c)
{
    auto [result, warning] = parse_mxs_config_text_to_map(c.zConfig);

    int errors = 0;
    bool has_global = result.config.count(CN_MAXSCALE) > 0;
    bool has_error = !result.errors.empty();
    bool has_warning = !warning.empty();

    if (has_global != c.expect_global_section)
    {
        cerr << "error: " << c.zName << ": expected the global section to be "
             << (c.expect_global_section ? "present" : "absent") << ", but it was not." << endl;
        ++errors;
    }

    if (has_error != c.expect_error)
    {
        cerr << "error: " << c.zName << ": expected " << (c.expect_error ? "an error" : "no error")
             << ", got: " << (result.errors.empty() ? string("none") : result.errors[0]) << endl;
        ++errors;
    }

    if (has_warning != c.expect_warning)
    {
        cerr << "error: " << c.zName << ": expected " << (c.expect_warning ? "a warning" : "no warning")
             << ", got: '" << warning << "'" << endl;
        ++errors;
    }

    // The old section must not survive under its old name, or later code would treat it as an
    // object definition without a type. This only has to hold when the file is accepted: a
    // rejected one is not renamed, and nothing reads it.
    if (!has_error && result.config.count(CN_MAXSCALE_LEGACY) > 0)
    {
        cerr << "error: " << c.zName << ": the section is still named '" << CN_MAXSCALE_LEGACY
             << "' after parsing." << endl;
        ++errors;
    }

    return errors;
}
}

int main(int argc, char* argv[])
{
    mxb::Log log;

    mxs::Config::init(argc, argv);

    int errors = 0;

    for (const auto& c : test_cases)
    {
        errors += test(c);
    }

    return errors;
}
