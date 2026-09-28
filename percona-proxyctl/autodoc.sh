#!/bin/bash

# Install dependencies
npm i

COMMANDS=$(
for i in `node percona-proxyctl.js --help|awk '/^$/{p=0} {if(p){print $2}}/Commands:/{p=1}'`
do
    echo "## $i"
    echo

    for j in `node percona-proxyctl.js --help $i|awk '/^$/{p=0} {if(p){print $3}}/Commands:/{p=1}'`
    do
        echo "### $i $j"
        echo
        echo \`\`\`
        echo "`node percona-proxyctl.js --help $i $j`"
        echo \`\`\`
        echo
    done
done
)

cat <<EOF > ../Documentation/Reference/Percona Proxyctl.md
# Percona Proxyctl

Percona Proxyctl is a command line administrative client for Percona Proxy which uses
the Percona Proxy REST API for communication. It has replaced the legacy MaxAdmin
command line client that is no longer supported or included.

By default, the Percona Proxy REST API listens on port 8989 on the local host. The
default credentials for the REST API are \`admin:mariadb\`. The users used by the
REST API are the same that are used by the MaxAdmin network interface. This
means that any users created for the MaxAdmin network interface should work with
the Percona Proxy REST API and Percona Proxyctl.

For more information about the Percona Proxy REST API, refer to the
[REST API documentation](../REST-API/API.md) and the
[Configuration Guide](../Getting-Started/Configuration-Guide.md).

[TOC]

# Limitations

* Percona Proxyctl does not work when used from a SystemD unit with MemoryDenyWriteExecute=true.

# .percona-proxyctl.cnf

If the file \`~/.percona-proxyctl.cnf\` exists, percona-proxyctl will use any values in the
section \`[percona-proxyctl]\` as defaults for command line arguments. For instance,
to avoid having to specify the user and password on the command line,
create the file \`.percona-proxyctl.cnf\` in your home directory, with the following
content:
\`\`\`
[percona-proxyctl]
u = my-name
p = my-password
\`\`\`
Note that all access rights to the file must be removed from everybody else
but the owner. Percona Proxyctl refuses to use the file unless the rights have been
removed.

Another file from which to read the defaults can be specified with the \`-c\`
flag.

# Commands

$COMMANDS

EOF
