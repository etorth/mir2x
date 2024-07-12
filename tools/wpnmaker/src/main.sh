#!/bin/bash

echo weapon graphics package builder
echo version 0.0.1

printUsage()
{
    echo "Usage:"
    echo "       main.sh gender index path-to-pkgviewer-output [path-to-output]"
    echo "       gender : 0 :   male"
    echo "                1 : female"
    echo "       index  : 1 : [w]m-weapon1.wil"
    echo "                2 : [w]m-weapon2.wil"
    echo "                3 : [w]m-weapon3.wil"
    echo "                4 : [w]m-weapon4.wil"
    echo "                5 : [w]m-weapon5.wil"
    echo "Example:"
    echo "      $ main.sh 1 2 /home/you/extract"
    echo "      $ main.sh 1 2 /home/you/extract /home/you/output"

    return 0
}

if (( $# != 3 && $# != 4 ))
then
    printUsage
    exit 1
fi

if (( $# == 3 ))
then
    p_gender=$1
     p_index=$2
      p_path=$(readlink -m $3)
       p_dst=$(readlink -m $(date +%Y%m%d%H%M%S%N))
fi

if (( $# == 4 ))
then
    p_gender=$1
     p_index=$2
      p_path=$(readlink -m $3)
       p_dst=$(readlink -m $4)
fi

# check arguments
if (( p_gender != 0 && p_gender != 1 ))
then
    echo invalid gender
    printUsage
    exit 1
fi

if (( p_index != 1 && p_index != 2 && p_index != 3 && p_index != 4 && p_index != 5))
then
    echo invalid index
    printUsage
    exit 1
fi

if [ ! -d $p_path ]
then
    echo $p_path is not a valid directory
    printUsage
    exit 1
fi

if [ -e $p_dst ]
then
    if [ ! -f $p_dst ]
    then
        echo $p_dst is not a valid directory
    fi
else
    mkdir -p $p_dst
fi

src_fd=$(readlink -m $p_path)
dst_fd=$(readlink -m $p_dst )

echo src: $src_fd
echo dst: $dst_fd

for fileName in $src_fd/TMP*_*.PNG
do
    # format of index
    # 3000 * GfxSet + 80 * State + 10 * Direction + (Frame + 1)

    imgIdx=`echo $fileName | sed 's/^.*\/TMP//'    - | sed 's/_.*$//'  - | sed 's/^0*//' -`
    imgOff=`echo $fileName | sed 's/^.*\/TMP.*_//' - | sed 's/\.PNG//' -`

    nGfxSet=$((    ($imgIdx - 1) / 3000                  ))
    nStatus=$((   (($imgIdx - 1) % 3000) / 80            ))
    nDirect=$((  ((($imgIdx - 1) % 3000) % 80) / 10      ))
    nFramen=$(( (((($imgIdx - 1) % 3000) % 80) % 10) / 1 ))

    # create the image index string, each wil file has 0 ~ 9 weapons
    # set nGlobalGfxSet = (p_index - 1) * 10 + nGfxSet --> [0, 49]
    szEncode=`printf "%08X" $(( ($p_gender  << 22)  \
        | ((($p_index - 1) * 10 + $nGfxSet) << 14)  \
        | ($nStatus << 8)                           \
        | ($nDirect << 5)                           \
        | ($nFramen << 0) ))`

    cp $fileName $dst_fd/$szEncode$imgOff.PNG

    echo "$imgIdx -> ($nGfxSet -> $(( ($p_index - 1) * 10 + $nGfxSet )), $nStatus, $nDirect, $nFramen) -> $szEncode"
    echo "$imgIdx -> ($nGfxSet -> $(( ($p_index - 1) * 10 + $nGfxSet )), $nStatus, $nDirect, $nFramen) -> $szEncode" >> $dst_fd/main.log
done

# done the rename and make the package
echo
echo done in folder $dst_fd
