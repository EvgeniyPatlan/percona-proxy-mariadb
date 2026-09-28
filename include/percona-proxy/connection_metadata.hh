/*
 * Copyright (c) 2023 MariaDB plc
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

#include <percona-proxy/ccdefs.hh>

#include <map>

namespace percona_proxy
{
// Binary version of a value in information_schema.COLLATIONS
struct Collation
{
    std::string collation;
    std::string character_set;
};

// Struct containing the metadata that Percona Proxy generates for the handshake
struct ConnectionMetadata
{
    std::map<std::string, std::string> metadata;
    std::map<int, Collation>           collations;
};
}
