#!/bin/bash

if [[ $# != 1 ]]
then
    echo "Usage: mir2x-client.sh build-path"
    exit 1
fi

env \
    CPUPROFILE=./mir2x-client.log       \
    LD_PRELOAD=/usr/lib/libprofiler.so  \
    MIR2X_DEBUG_SHOW_MAP_GRID=1         \
    MIR2X_DEBUG_SHOW_LOCATION=1         \
    MIR2X_DEBUG_SHOW_CREATURE_COVER=1   \
    $1/client/src/client

google-pprof --callgrind $1/client/src/client ./mir2x-client.log > ./mir2x-client.callgrind
kcachegrind ./mir2x-client.callgrind
