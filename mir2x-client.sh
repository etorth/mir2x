#!/bin/bash

if [[ $# != 1 ]]
then
    echo "Usage: mir2x-client.sh build-path"
    exit 1
fi

env MIR2X_DEBUG_SHOW_MAP_GRID=1 MIR2X_DEBUG_SHOW_CREATURE_COVER=1 $1/client/src/client
