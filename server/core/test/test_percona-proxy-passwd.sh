#!/bin/bash

# Create a temporary directory: /tmp/tmp.<random>
SECRETS_DIR=`mktemp -d`

# Generate a .secrets file.
../percona-proxy-keys -u $(whoami) ${SECRETS_DIR} || exit 1
rv=0

for ((i=0;i<10;i++))
do
    RES=$(../percona-proxy-passwd ${SECRETS_DIR} -d $(../percona-proxy-passwd ${SECRETS_DIR} dummy))

    if [ "$RES" != "dummy" ]
    then
        echo "Encrypting 'dummy' decrypted into '$RES'"
        rv=1
        break
    fi
done

# Remove the temporary directory.
rm -rf ${SECRETS_DIR}

exit $rv
