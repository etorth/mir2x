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

    return math.ceil(1000 / statistics.mean([x.info['duration'] for x in imglist]))


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
            outName = "out." + str(index) + ".png"
            simpleMerge(gifRowNameList, outName, True)
            gifMergedRowNameList.append(outName)

    # create name to load in zsdb
    # check client/src/emoticondb.hpp see how name get formated
    outFileName = "%06X%02X%02X%04X%04X%04X.PNG" % (emojiID, frameCount, fps, frameWidth, frameHeight, frameHeight)
    simpleMerge(gifMergedRowNameList, outFileName, False)


def doConvert(gifOrigName, emojiID):
    if not os.path.exists(gifOrigName):
        raise ValueError("%s doesn't exists" % gifOrigName)

    if (emojiID < 0) or (emojiID >= 0XFFFFFF):
        raise ValueError('invalid output emoji id: %d', emojiID)

    # decompose gif into png files
    # "-coalesce" helps to make good frames, otherwise some frames can be partial because of optimization
    subprocess.run(["convert", "-coalesce", gifOrigName, gifOrigName + ".frame.%04d.png"])

    frameFileList = glob.glob(gifOrigName + '.frame.*.png')
    fps = getFPS(gifOrigName)

    mergeFrames(frameFileList, fps, emojiID)


def main():
    # convert gif to png based emoji
    # use ImageMagic command line tool: convert, because it works better for decompse/merge

    # or use ezGIF
    # or check this link: https://gist.github.com/BigglesZX/4016539

    # usage
    # output needs an ID for emojiDB
    if len(sys.argv) != 3:
        raise ValueError("usage: python3 gif2emoji_convert.py file.gif 1230")

    doConvert(sys.argv[1], int(sys.argv[2]))


if __name__ == "__main__":
    main()
