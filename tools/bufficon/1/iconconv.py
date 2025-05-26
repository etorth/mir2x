import os
import sys
from PIL import Image


def cropOneLine(lineIndex, iconFile, startX, startY, width, height, offset, outDir):
    img = Image.open(iconFile, 'r')

    index = 0
    imgWidth, imgHeight = img.size

    while startX < imgWidth:
        subImg = img.crop((startX, startY, startX + width, startY + height)).convert('RGBA')

        subImg.putpixel(( 0,  0), (0, 0, 0, 0))
        subImg.putpixel((51,  0), (0, 0, 0, 0))
        subImg.putpixel(( 0, 51), (0, 0, 0, 0))
        subImg.putpixel((51, 51), (0, 0, 0, 0))

        subImg = subImg.resize((46, 46), Image.ANTIALIAS)
        subImg.save('%s/%s_%d_%d.png' % (outDir, iconFile, lineIndex, index))

        index += 1
        startX += offset


def iconconv(iconFile, outDir):
    if not os.path.exists(iconFile):
        raise ValueError("%s doesn't exists" % iconFile)

    if not os.path.exists(outDir):
        raise ValueError("%s doesn't exists" % outDir)

    cropOneLine(0, iconFile, 9,  18, 52, 52, 72, outDir)
    cropOneLine(1, iconFile, 9,  89, 52, 52, 72, outDir)
    cropOneLine(2, iconFile, 9, 159, 52, 52, 72, outDir)
    cropOneLine(3, iconFile, 9, 230, 52, 52, 72, outDir)
    cropOneLine(4, iconFile, 9, 301, 52, 52, 72, outDir)
    cropOneLine(5, iconFile, 9, 372, 52, 52, 72, outDir)


def main():
    # need to install pillow:
    # python3 -m pip install --upgrade Pillow

    if len(sys.argv) != 2:
        raise ValueError("usage: python3 iconconv.py <out-dir>")
    iconconv('1.png', sys.argv[1])
    iconconv('2.png', sys.argv[1])

if __name__ == "__main__":
    main()
