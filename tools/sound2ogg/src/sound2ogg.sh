#!/bin/bash

if test $# -ne 1
then
    echo "Usage: sound2ogg.sh <input-dir>"
    exit 1
fi

if [ ! -d $1 ]
then
    echo "Not a directory: $1"
    exit 1
fi

if which dir2ogg > /dev/null
then
    echo "Found dir2ogg"
else
    echo "Install dir2ogg: sudo apt-get install dir2ogg"
    exit 1
fi

# dir2ogg doesn't provide a output directory parameter
# all output will be in input directory, this may overwrite existing .ogg files

for wavfile in $1/*.[Ww][Aa][Vv]
do
    dir2ogg $wavfile -q 10 # use best quality
done

for mp3file in $1/*.[Mm][Pp]3
do
    dir2ogg $mp3file -t # use original mp3 quality
done
