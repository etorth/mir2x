#!/bin/bash

# use a provided string -> id converter
# then convert all map to the id and store in a zip file
# usage:
#        main.sh path/to/files  path/to/converter

function printUsage()
{
    echo "Usage:"
    echo "      main.sh path/to/mapfiles_folder path/to/converter"
    echo "# 1. all needed map files with extension .map in specified folder"
    echo "# 2. converter accepts basename only and covert to mapID as %08X"
}

if [[ $# != 2 ]]
then
    printUsage
    exit 1
fi

rm -f MapDB.ZIP

for mapFileFullName in `find $1`
do
    if [ -f $mapFileFullName ]
    then
        if [[ $mapFileFullName =~ .*\.map$ ]]
        then
            mapFileName=`basename $mapFileFullName | sed 's/\..*$//' -`
            mapFileCode=`$2 $mapFileName`

            if [[ ${#mapFileCode} != 8 ]]
            then
                echo "warning: $mapFileFullName -> $mapFileCode"
                continue
            fi

            cp $mapFileFullName /tmp/$mapFileCode.MAP
            printf "info: $mapFileFullName -> $mapFileCode" && zip -j MapDB.ZIP /tmp/$mapFileCode.MAP
        fi
    fi
done
