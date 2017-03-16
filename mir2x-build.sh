#!/bin/bash

BuildLinux()
{
    mkdir -p build/linux
}

BuildAndroid()
{
    mkdir -p build/android
}

BuildWindows()
{
    mkdir -p build/windows
}

BuildClean()
{
    # we put all build files in build/*
    rm build -rf
}

if [[ $# == 1 ]]
then
    if [[ $1 == "--help" ]]
    then
        echo "Usage: mir2x-build.sh"
        echo "       mir2x-build.sh --help"
        exit 0
    else
        echo Invalid option: $1
        exit 1
    fi
fi

# OK script issued without arguments
# try to detect all supported target and build them all

exit 0
