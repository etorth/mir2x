#!/bin/bash

echo '+-------------------------------------------------------+'
echo '|  monster graphics package builder                     |'
echo '|  based on binaries: https://github.com/etorth/CBWCQ3  |'
echo '+-------------------------------------------------------+'

if [[ $# != 1 && $# != 2 ]]
then
    echo "Usage:"
    echo "    main.sh <mir2ei_install_prefix> [prefix_length]"
    exit 1
fi

mir2ei_install_prefix="$( readlink -m $1 )"
script_install_prefix="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

if [ -z "$2" ]
then
    prefix_length=$2
else
    prefix_length=0
fi

if [ ! -d $mir2ei_install_prefix ]
then
    echo bad mir3 install dir: $mir2ei_install_prefix
    exit 1
fi

echo mir2ei install prefix: $mir2ei_install_prefix
echo script install prefix: $script_install_prefix

output_prefix=$(date +%Y%m%d%H%M%S%N)

for index in $( seq 0 19 )
do
    file_index=$(( index + 1 ))
    output_dir=$output_prefix/mon_$index

    body_file=$mir2ei_install_prefix/Mon-$file_index.wil
    shadow_file=$mir2ei_install_prefix/MonS-$file_index.wil

    if [ ! -f $body_file ]
    then
        echo file not found: $body_file
        continue
    fi

    if [ ! -f $shadow_file ]
    then
        echo file not found: $body_file
        continue
    fi

    echo create $output_dir

    mkdir -p $output_dir
    $script_install_prefix/monsterwil2png $index $mir2ei_install_prefix Mon-$file_index wil MonS-$file_index wil $output_dir $prefix_length
done
