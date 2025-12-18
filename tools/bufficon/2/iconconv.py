import os
import sys
from PIL import Image


def cropOneLine(lineIndex, iconFile, frameFile, startX, startY, width, height, offset, addFrame, outDir):
    img = Image.open(iconFile, 'r')
    frameImg = Image.open(frameFile, 'r')

    index = 0
    imgWidth, imgHeight = img.size

    while startX < imgWidth:
        subImg = img.crop((startX, startY, startX + width, startY + height)).convert('RGBA')

        subImg.putpixel(( 0,  0), (0, 0, 0, 0))
        subImg.putpixel((45,  0), (0, 0, 0, 0))
        subImg.putpixel(( 0, 45), (0, 0, 0, 0))
        subImg.putpixel((45, 45), (0, 0, 0, 0))

        if addFrame:
            subImg.paste(frameImg, (0, 0), frameImg.convert('RGBA'))
        subImg.save('%s/%d_%d.png' % (outDir, lineIndex, index))
        index += 1
        startX += offset


def iconconv(iconFile, frameFile, addFrame, outDir):
    if not os.path.exists(iconFile):
        raise ValueError("%s doesn't exists" % iconFile)

    if not os.path.exists(outDir):
        raise ValueError("%s doesn't exists" % outDir)

    cropOneLine(0, iconFile, frameFile, 7,  14, 46, 46, 50, addFrame, outDir)
    cropOneLine(1, iconFile, frameFile, 7,  98, 46, 46, 50, addFrame, outDir)
    cropOneLine(2, iconFile, frameFile, 7, 148, 46, 46, 50, addFrame, outDir)
    cropOneLine(3, iconFile, frameFile, 7, 241, 46, 46, 50, addFrame, outDir)
    cropOneLine(4, iconFile, frameFile, 7, 291, 46, 46, 50, addFrame, outDir)
    cropOneLine(5, iconFile, frameFile, 7, 341, 46, 46, 50, addFrame, outDir)
    cropOneLine(6, iconFile, frameFile, 7, 391, 46, 46, 50, addFrame, outDir)
    cropOneLine(7, iconFile, frameFile, 7, 486, 46, 46, 50, addFrame, outDir)
    cropOneLine(8, iconFile, frameFile, 7, 579, 46, 46, 50, addFrame, outDir)
    cropOneLine(9, iconFile, frameFile, 7, 666, 46, 46, 50, addFrame, outDir)


def main():
    # need to install pillow:
    # python3 -m pip install --upgrade Pillow

    # used to convert 1.png to icons, add frame by 2.png
    if len(sys.argv) != 3:
        raise ValueError("usage: python3 iconconv.py <add-frame: 0/1> <out-dir>")
    iconconv('1.png', '2.png', int(sys.argv[1]), sys.argv[2])

if __name__ == "__main__":
    main()
