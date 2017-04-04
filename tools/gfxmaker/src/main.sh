#!/bin/bash

echo hero graphics package builder
echo version 0.01

if [[ $# != 1 ]]
then
    echo "Usage:"
    echo "       main.sh path-to-pkgviewer-output"
    exit 1
fi

src_fd=$(readlink -m $1)
dst_fd=$(readlink -m $(date +%Y%m%d%H%M%S%N))
if [ -d $source_folder ]
then
    echo src: $src_fd
    echo dst: $dst_fd
fi

