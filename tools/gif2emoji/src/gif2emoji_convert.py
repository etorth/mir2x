import os
import sys
import math
import glob
import subprocess
import statistics
from PIL import Image


def clearFrameWhiteBorder(mergedFileName):
    if not os.path.exists(mergedFileName):
        raise ValueError("%s doesn't exists" % mergedFileName)

    img = Image.open(mergedFileName, 'r')
    if img.mode in ('RGBA', 'LA') or (img.mode == 'P' and 'transparency' in img.info):
        img = img.convert('RGBA')
        r, g, b, a = img.split()

        alphaThreshold0 = 100
        alphaThreshold1 = 200

        # create edge mask binary image
        # I noticed fortunately the white border is always 1-pixel width
        edgeMask = Image.new('1', img.size, 0)
        for y in range(img.size[1]):
            for x in range(img.size[0]):
                if a.getpixel((x, y)) > alphaThreshold1:
                    for dy in [-1, 0, 1]:
                        for dx in [-1, 0, 1]:
                            if x + dx >= 0 and x + dx < img.size[0] and y + dy >= 0 and y + dy < img.size[1] and a.getpixel((x + dx, y + dy)) < alphaThreshold0:
                                edgeMask.putpixel((x, y), 1)

        # this loops changes the color/alpha pixel on edge
        # after this the ``a" is not 0/255 binary
        aOrig = a.copy()
        for y in range(img.size[1]):
            for x in range(img.size[0]):
                if a.getpixel((x, y)) < alphaThreshold0:
                    r.putpixel((x, y), 0)
                    g.putpixel((x, y), 0)
                    b.putpixel((x, y), 0)

                if edgeMask.getpixel((x, y)) == 1:
                    if r.getpixel((x, y)) + g.getpixel((x, y)) + b.getpixel((x, y)) > 230 * 3:
                        r.putpixel((x, y), 5)
                        g.putpixel((x, y), 5)
                        b.putpixel((x, y), 5)
                        a.putpixel((x, y), 5)

        # create a stepping alpha channel
        # original alpha channel is either 0 or 255, this makes the sharp edge
        aAvg = a.copy()
        for y in range(img.size[1]):
            for x in range(img.size[0]):
                sum = 0
                sumFull = 0
                for dy in [-1, 0, 1]:
                    for dx in [-1, 0, 1]:
                        if x + dx >= 0 and x + dx < img.size[0] and y + dy >= 0 and y + dy < img.size[1]:
                            sum = sum + a.getpixel((x + dx, y + dy))
                            sumFull = sumFull + 255

                sum = sum + a.getpixel((x, y)) * 2
                sumFull = sumFull + 255 * 2

                aAvg.putpixel((x, y), min(255, round(255 * sum / sumFull)))

        for y in range(img.size[1]):
            for x in range(img.size[0]):
                img.putpixel((x, y), (r.getpixel((x, y)), g.getpixel((x, y)), b.getpixel((x, y)), aAvg.getpixel((x, y))))


        # still need to make alpha channel
        # i.e. classic/034.gif

        # guess the blending using alpha blend:
        #
        #       dstRGB = srcRGB * srcA + dstRGB * (1 - srcA)
        #
        # here the left dstRGB is what we can get from the loaded image
        # we need to guess dstRGB on right side and srcA to estimate the srcRGB, looks not very useful

        guess_origDstR = 253
        guess_origDstG = 253
        guess_origDstB = 253
        guess_origSrcA = 0.8

        for y in range(img.size[1]):
            for x in range(img.size[0]):
                if aOrig.getpixel((x, y)) > alphaThreshold1:
                    srcR = min(255, round((r.getpixel((x, y)) - guess_origDstR * (1.0 - guess_origSrcA)) / guess_origSrcA))
                    srcG = min(255, round((g.getpixel((x, y)) - guess_origDstG * (1.0 - guess_origSrcA)) / guess_origSrcA))
                    srcB = min(255, round((b.getpixel((x, y)) - guess_origDstB * (1.0 - guess_origSrcA)) / guess_origSrcA))

                    img.putpixel((x, y), (srcR, srcG, srcB, aAvg.getpixel((x, y))))

        img.save(mergedFileName)


def getFPS(gifOrigName):
    im = Image.open(gifOrigName, 'r')
    imglist = []

    try:
        index = 0
        while True:
            im.seek(index)
            imglist.append(im.copy())
            index += 1
    except EOFError:
        pass

    # TODO I found there are gifs with duration zero, i.e. client/bin/emoji/classic/132.gif
    mean = statistics.mean([x.info['duration'] for x in imglist])

    if mean <= 0:
        return 10

    return math.ceil(1000 / mean)


def simpleMerge(gifFrameNameList, outName, toRow):
    command = ["convert"]
    if toRow:
        command.append("+append")
    else:
        command.append("-append")

    command = command + gifFrameNameList
    command.append(outName)

    print(command)
    subprocess.run(command)


def mergeFrames(gifFrameNameList, fps, emojiID, clearBorder):
    frameCount = len(gifFrameNameList)
    if frameCount == 0:
        raise ValueError("empty frame list")

    firstFrame = Image.open(gifFrameNameList[0])
    frameWidth, frameHeight = firstFrame.size

    for img in gifFrameNameList:
        frame = Image.open(img)
        if (frameWidth != frame.size[0]) or (frameHeight != frame.size[1]):
            raise ValueError("image in list should have same size")

    square = frameWidth * frameHeight * len(gifFrameNameList)
    edgeSize = math.sqrt(square)

    gridWidth = math.ceil(edgeSize / frameWidth)
    gridHeight = (len(gifFrameNameList) + gridWidth - 1) // gridWidth

    gifMergedRowNameList = []
    index = 0

    for ih in range(gridHeight):
        gifRowNameList = []
        for iw in range(gridWidth):
            if index >= len(gifFrameNameList):
                break
            gifRowNameList.append(gifFrameNameList[index])
            index += 1

        if len(gifRowNameList) > 0:
            if len(gifRowNameList) < gridWidth:
                # not a full row
                # create transparent empty images and append to the list
                subprocess.run(["convert", "-size", "%dx%d" % (frameWidth, frameHeight), "xc:transparent", "empty.png"])
                while len(gifRowNameList) < gridWidth:
                    gifRowNameList.append("empty.png")

            outName = "out." + str(index) + ".png"
            simpleMerge(gifRowNameList, outName, True)
            gifMergedRowNameList.append(outName)

    # create name to load in zsdb
    # check client/src/emoticondb.hpp see how name get formated
    outFileName = "%06X00%02X%02X%04X%04X%04X.PNG" % (emojiID, frameCount, fps, frameWidth, frameHeight, frameHeight)
    simpleMerge(gifMergedRowNameList, outFileName, False)

    if clearBorder:
        clearFrameWhiteBorder(outFileName)


def doConvert(gifOrigFullName, emojiID, clearBorder):
    if not os.path.exists(gifOrigFullName):
        raise ValueError("%s doesn't exists" % gifOrigFullName)

    if (emojiID < 0) or (emojiID > 0XFFFFFF):
        raise ValueError('invalid output emoji id: %d', emojiID)

    gifOrigName = os.path.basename(gifOrigFullName)

    print("")
    print("converting " + gifOrigName)

    # decompose gif into png files
    # "-coalesce" helps to make good frames, otherwise some frames can be partial because of optimization
    subprocess.run(["convert", "-coalesce", gifOrigFullName, gifOrigName + ".frame.%04d.png"])

    frameFileList = glob.glob(gifOrigName + '.frame.*.png')
    fps = getFPS(gifOrigFullName)
    mergeFrames(frameFileList, fps, emojiID, clearBorder)


def doConvertDir(gifDir, emojiGroupID, clearBorder):
    if emojiGroupID < 0 or emojiGroupID > 0XFF:
        raise ValueError('invalid output emoji group id: %d', emojiGroupID)

    gifOrigFullNameList = []
    for file in os.listdir(gifDir):
        if os.path.isfile(gifDir + '/' + file) and (file.endswith('.gif') or file.endswith('.GIF')):
            gifOrigFullNameList.append(gifDir + '/' + file)

    print("detected %d gif files:" % len(gifOrigFullNameList))
    for gifOrigFullName in gifOrigFullNameList:
        print(gifOrigFullName)

    if len(gifOrigFullNameList) > 0XFFFF:
        raise ValueError('an emoji group can contain maximal 65535 emojis')

    index = 0
    for gifOrigFullName in gifOrigFullNameList:
        doConvert(gifOrigFullName, (emojiGroupID << 16) + index, clearBorder)
        index = index + 1


def main():
    # convert gifs in one dir to png based emoji
    # use ImageMagic command line tool: convert, because it works better for decompse/merge

    # or use ezGIF
    # or check this link: https://gist.github.com/BigglesZX/4016539

    # usage
    # output needs an ID for emojiDB

    # currently: client/bin/res/emoji/classic -> 0
    #            client/bin/res/emoji/new     -> 1
    if len(sys.argv) != 4:
        raise ValueError("usage: python3 gif2emoji_convert.py gif-dir-path emojiGroupID clear-white-border")

    doConvertDir(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))


if __name__ == "__main__":
    main()
