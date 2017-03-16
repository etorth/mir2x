#!/bin/bash

BuildLinuxClient()
{
    mkdir -p build/linux/client
}

BuildLinuxServer()
{
    mkdir -p build/linux/server
}

BuildLinux()
{
    BuildLinuxClient
    BuildLinuxServer
    BuildAndroidClient
}

BuildWindowsClient()
{
    mkdir -p build/windows/client
}

BuildWindowsServer()
{
    mkdir -p build/windows/server
}

BuildWindows()
{
    BuildWindowsClient
    BuildWindowsServer
    BuildAndroidClient
}

BuildAndroidClient()
{
    mkdir -p build/android/client
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
