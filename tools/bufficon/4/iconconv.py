import os
import sys
import glob
from PIL import Image


def iconconv(frameFile, outDir):
    if not os.path.exists(frameFile):
        raise ValueError("%s doesn't exists" % frameFile)

    if not os.path.exists(outDir):
        raise ValueError("%s doesn't exists" % outDir)

    index = 1
    frameImg = Image.open('0.png', 'r').convert('RGBA')

    for iconFile in glob.glob('*.png'):
        if iconFile == frameFile:
            continue

        img = Image.open(iconFile, 'r').convert('RGBA').resize((46, 46), Image.ANTIALIAS)
        img.putpixel(( 0,  0), (0, 0, 0, 0))
        img.putpixel((45,  0), (0, 0, 0, 0))
        img.putpixel(( 0, 45), (0, 0, 0, 0))
        img.putpixel((45, 45), (0, 0, 0, 0))
        img.paste(frameImg, (0, 0), frameImg)
        img.save('%s/out_%d.png' % (outDir, index))
        index += 1


def main():
    # need to install pillow:
    # python3 -m pip install --upgrade Pillow

    if len(sys.argv) != 2:
        raise ValueError("usage: python3 iconconv.py <out-dir>")
    iconconv('0.png', sys.argv[1])


if __name__ == "__main__":
    main()
