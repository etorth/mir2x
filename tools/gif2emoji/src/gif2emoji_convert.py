import os
import sys
import math
import glob
import subprocess
import statistics
from PIL import Image


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
        return 7

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


def mergeFrames(gifFrameNameList, fps, emojiID):
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


def doConvert(gifOrigFullName, emojiID):
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
    mergeFrames(frameFileList, fps, emojiID)


def doConvertDir(gifDir, emojiGroupID):
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
        doConvert(gifOrigFullName, (emojiGroupID << 16) + index)
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
    if len(sys.argv) != 3:
        raise ValueError("usage: python3 gif2emoji_convert.py gif-dir-path emojiGroupID")

    doConvertDir(sys.argv[1], int(sys.argv[2]))


if __name__ == "__main__":
    main()
