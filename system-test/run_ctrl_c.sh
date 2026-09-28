#!/bin/bash
#
# Copyright (c) 2016 MariaDB Corporation Ab
# Copyright (c) 2023 MariaDB plc, Finnish Branch
#
# Use of this software is governed by the Business Source License included
# in the LICENSE.TXT file and at www.mariadb.com/bsl11.
#
# Change Date: 2026-09-21
#
# On the date above, in accordance with the Business Source License, use
# of this software will be governed by version 2 or later of the General
# Public License.
#

###
## @file run_ctrl_c.sh
## check that Percona Proxy is reacting correctly on ctrc+c signal and termination does not take ages
set -x
scp -i ${percona_proxy_000_keyfile} -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -r $src_dir/test_ctrl_c/* ${percona_proxy_000_whoami}@${percona_proxy_000_network}:./
ssh -i ${percona_proxy_000_keyfile} -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no ${percona_proxy_000_whoami}@${percona_proxy_000_network} "export percona_proxy_000_access_sudo=${percona_proxy_000_access_sudo}; ./test_ctrl_c.sh"
res=$?

exit $res
