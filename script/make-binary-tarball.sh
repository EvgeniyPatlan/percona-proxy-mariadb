#!/bin/sh


read -p "Enter path where Percona Proxy is installed:" instpath
if [ "${instpath}" = "" ]; then
		echo "Error: input path is null, exit"
		exit 1
fi

BINARY_PATH=${instpath}
cd ${BINARY_PATH}
BINARY_PATH=${PWD}
echo "Looking for Percona Proxy in [${BINARY_PATH}]"

if [ -s "${BINARY_PATH}/bin/percona-proxy" ]; then
	if [ -x "${BINARY_PATH}/bin/percona-proxy" ]; then
		PERCONA_PROXY_VERSION=`strings ${BINARY_PATH}/bin/percona-proxy | grep "MariaDB Corporation Percona Proxy" | awk '{print $3}' | head -1`
		echo "Found Percona Proxy, version: ${PERCONA_PROXY_VERSION}"
	fi
else
	echo "Error: Percona Proxy was not found!"
	exit 1
fi

PERCONA_PROXY_BINARY_TARFILE=percona-proxy.${PERCONA_PROXY_VERSION}.tar
TARFILE_BASEDIR=percona-proxy-${PERCONA_PROXY_VERSION}
TARFILE_BASEDIR_SUBST='s,^\.,'${TARFILE_BASEDIR}','

rm -rf ${PERCONA_PROXY_BINARY_TARFILE}.gz
rm -rf ${PERCONA_PROXY_BINARY_TARFILE}

TARFILE_BASEDIR_SUBST='s,^'${BINARY_PATH}','${TARFILE_BASEDIR}','

tar --absolute-names --owner=percona-proxy --group=percona-proxy --transform=${TARFILE_BASEDIR_SUBST} -cf ${PERCONA_PROXY_BINARY_TARFILE} ${BINARY_PATH}/*
gzip ${PERCONA_PROXY_BINARY_TARFILE}

if [ -s "${PERCONA_PROXY_BINARY_TARFILE}.gz" ]; then
	echo "File ["${PERCONA_PROXY_BINARY_TARFILE}".gz] is ready in ["$BINARY_PATH"]"
else
	echo "Error: File ["${PERCONA_PROXY_BINARY_TARFILE}".gz] was not created in ["$BINARY_PATH"]"
fi
