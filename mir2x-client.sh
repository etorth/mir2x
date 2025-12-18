#!/bin/bash

if [[ $# != 1 ]]
then
    echo "Usage: mir2x-client.sh install-path"
    exit 1
fi

BIN_DIR=$1
LOG_DIR=${PWD}

echo "Profiling client installed at: ${BIN_DIR}"
echo "Profiling result generated at: ${LOG_DIR}"

(\
    cd ${BIN_DIR};  \
    env             \
    CPUPROFILE=${LOG_DIR}/mir2x-client.log  \
    LD_PRELOAD="/usr/lib/libprofiler.so"    \
    ./client                                \
)

google-pprof --svg ${BIN_DIR}/client ${LOG_DIR}/mir2x-client.log > ${LOG_DIR}/mir2x-client.svg
