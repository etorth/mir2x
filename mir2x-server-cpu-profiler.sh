#!/bin/bash

if [[ $# != 1 ]]
then
    echo "Usage: $0 install-path"
    exit 1
fi

BIN_DIR=$1
LOG_DIR=${PWD}

echo "Profiling server installed at: ${BIN_DIR}"
echo "Profiling result generated at: ${LOG_DIR}"

cd ${BIN_DIR};
env CPUPROFILE=${LOG_DIR}/server.cpu.log LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libtcmalloc_and_profiler.so.4.6.11" ./server --auto-launch --disable-monster-idle-wait

cd ${LOG_DIR};
google-pprof --svg ${BIN_DIR}/server ${LOG_DIR}/mir2x-server.log > ${LOG_DIR}/mir2x-server.svg
