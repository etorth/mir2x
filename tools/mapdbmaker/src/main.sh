#!/bin/bash

# use a provided string -> id converter
# then convert all map to the id and store in a zip file
# usage:
#        main.sh path/to/mir2xmap  path/to/converter

function printUsage()
{
    echo "Usage:"
    echo "      main.sh path/to/mir2xmap path/to/converter"
    echo "# 1. exam all needed map package in mir2xmap: DESC.BIN and IMG"
    echo "# 2. converter accepts basename only and covert to mapID as %08X"
    echo "# 3. create two dbs: MapDBN.ZIP and MapBinDBN.ZIP"
}

if [[ $# != 2 ]]
then
    printUsage
    exit 1
fi

rm -f MapDBN.ZIP
rm -f MapBinDBN.ZIP

for mapModuleName in `ls $1`
do
    # skip if not folders
    if [ ! -d $1/$mapModuleName ]
    then
        continue
    fi

    # skip if not map folders
    mapModuleCode=$($2 $(basename $mapModuleName))

    if [[ ${#mapModuleCode} != 8 ]]
    then
        echo "WARNING: $mapModuleName -> $mapModuleCode"
        continue
    fi

    # skip and warning if not valid map folders
    if [ ! -f $1/$mapModuleName/DESC.BIN ]
    then
        echo "WARNING: $mapModuleName: no DESC.BIN found"
        continue
    fi

    if [ ! -d $1/$mapModuleName/IMG ]
    then
        echo "WARNING: $mapModuleName: no IMG folder found"
        continue
    fi

    # ok

    echo $mapModuleName : update MapDBN.ZIP
    ls $1/$mapModuleName/IMG/*.PNG | xargs zip -j MapDBN.ZIP > /dev/null
    
    echo $mapModuleName : update MapBinDBN.ZIP
    cp -f $1/$mapModuleName/DESC.BIN $mapModuleCode.MAP
    printf "INFO: $mapModuleName -> $mapModuleCode" && zip -j MapBinDBN.ZIP $mapModuleCode.MAP
    rm -f $mapModuleCode.MAP
done
