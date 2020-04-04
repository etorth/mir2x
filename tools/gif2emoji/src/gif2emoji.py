import os
import sys
import math
from PIL import Image
import statistics

def gif2PNGSheet(gifName):
    im = Image.open(gifName, 'r')
    imglist = []

    try:
        index = 0
        while True:
            im.seek(index)
            imglist.append(im.copy())
            index += 1
    except EOFError:
        pass

    return mergeFrames(imglist)

def mergeFrames(imglist):
    if(len(imglist) == 0):
        raise ValueError("empty image list")

    imgWidth  = imglist[0].size[0]
    imgHeight = imglist[0].size[1]

    for img in imglist:
        if (imgWidth != img.size[0]) or (imgHeight != img.size[1]):
            raise ValueError("image in list should have same size")

    fps = math.ceil(1000 / statistics.mean([x.info['duration'] for x in imglist]))
    widthCount  = math.ceil(math.sqrt(len(imglist)))
    heightCount = (len(imglist) + widthCount - 1) // widthCount
    mergedImg = Image.new("RGBA", (widthCount * imgWidth, heightCount * imgHeight), (0, 0, 0, 0))

    index = 0
    for ih in range(heightCount):
        for iw in range(widthCount):
            if index >= len(imglist):
                break

            print(len(imglist), iw, ih)

            dstX = imgWidth  * iw + int((imgWidth  - imglist[index].size[0]) / 2)
            dstY = imgHeight * ih + int((imgHeight - imglist[index].size[1]) / 2)

            mergedImg.paste(imglist[index], (dstX, dstY))
            index += 1

    return mergedImg, len(imglist), imgWidth, imgHeight, fps, imglist[0].info['transparency']

def gif2emoji(gifName, outId):
    if not os.path.exists(gifName):
        raise ValueError("%s doesn't exists" % gifName)

    if (outId < 0) or (outId >= 0XFFFFFF):
        raise ValueError('invalid output emoji id: %d', outId)

    # create name to load in zsdb
    # check client/src/emoticondb.hpp see how name get formated

    mergedImg, frameCount, frameWidth, frameHeight, fps, transp = gif2PNGSheet(gifName)
    outfileName = "%06X%02X%02X%04X%04X%04X.PNG" % (outId, frameCount, fps, frameWidth, frameHeight, frameHeight)
    # mergedImg.save(outfileName, transparency=transp)
    mergedImg.save(outfileName)

def main():
    # I use:
    # python3 gif2emoji.py fileName.gif 123
    # need to install: python3 -m pip install --upgrade Pillow
    if len(sys.argv) != 3:
        raise ValueError("usage: gif2emoji fileName.gif 123")
    gif2emoji(sys.argv[1], int(sys.argv[2]))

if __name__ == "__main__":
    main()
