#!/bin/bash

if test $# -ne 2
then
    echo "Usage: sound2ogg.sh <input-dir> <output-dir>"
    exit 1
fi

if [ ! -d $1 ]
then
    echo "Invalid input directory: $1"
    exit 1
fi

if [ ! -d $2 ]
then
    echo "Invalid output directory: $2"
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

# prevent the wildcard file name itself becomes a match
# be careful of using this
shopt -s nullglob

for oggfile in $1/*.[Oo][Gg][Gg]
do
    echo "WARNING: detected .ogg file in input directory: $oggfile"
    hasOGGFile=1
done

if [ ! -z $hasOGGFile ]
then
    exit 1
fi

for wavfile in $1/*.[Ww][Aa][Vv]
do
    dir2ogg $wavfile -q 10 -Q # use best quality
done

for mp3file in $1/*.[Mm][Pp]3
do
    dir2ogg $mp3file -t -Q # use original mp3 quality
done

# done conversion
# move all .ogg files to output directory

for oggfile in $1/*.[Oo][Gg][Gg]
do
    mv -f $oggfile $2
done
