#!/bin/bash

if [[ $# < 1 ]]
then
    echo "Usage: $0 server-bin-path [options]"
    exit 1
fi

if [[ ! -f "$1" ]]
then
    echo "Error: invalid server binary: $1"
    exit 1
fi

LOG_DIR=${PWD}
BIN_DIR=$(dirname "$1")
BIN_NAME=$(basename "$1")

shift

if [[ ! -d "${BIN_DIR}" ]]
then
    echo "Error: invalid server-install-path: ${BIN_DIR}"
    exit 1
fi


if [[ -z "${PROF_LIB_PATH}" || ! -f "${PROF_LIB_PATH}" ]]
then
    echo "Error: PROF_LIB_PATH is not set or does not point to a valid directory"
    exit 1
fi

echo "Profiling result generated at: ${LOG_DIR}"
echo "Profiling server installed at: ${BIN_DIR}"

cd ${BIN_DIR};
env CPUPROFILE=${LOG_DIR}/server.cpu.log LD_PRELOAD="${PROF_LIB_PATH}" ./${BIN_NAME} "$@"

cd ${LOG_DIR};
google-pprof --svg ${BIN_DIR}/server ${LOG_DIR}/server.cpu.log > ${LOG_DIR}/server.cpu.svg
