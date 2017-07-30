#!/bin/bash

if [[ $# != 1 ]]
then
    echo "Usage: mir2x-server.sh build-path"
    exit 1
fi

env \
    CPUPROFILE=./mir2x-server.log       \
    LD_PRELOAD=/usr/lib/libprofiler.so  \
    $1/server/monoserver/src/monoserver

google-pprof --callgrind $1/server/monoserver/src/monoserver ./mir2x-server.log > ./mir2x-server.callgrind
kcachegrind ./mir2x-server.callgrind
