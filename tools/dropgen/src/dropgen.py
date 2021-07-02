import os
import re
import sys
import math
import glob
import subprocess

# global re pattern
# compile it as use all times
g_item = re.compile('^\s*(\d+)/(\d+)\s+(\S+)\s*$')
#                     --- --- - ---     ---
#                      ^   ^  ^  ^       ^
#                      |   |  |  |       |
#                      |   |  |  |       +--- item name
#                      |   +--+--+----------- a / b
#                      +--------------------- optional comment markers

def printCode(s):
    print('--->%s' % s)

def dropGenOneMonster(name, file):
    with open(file, 'r', encoding='gb2312') as fp:
        for line in fp:
            match = g_item.search(line.strip())
            if match:
                printCode('{%s, %s, 0, %s},' % (name, match.group(3), match.group(2)))


def dropGen(dir):
    monDict = {}
    for file in os.listdir(dir):
        if os.path.isfile(dir + '/' + file) and (file.endswith('.txt') or file.endswith('.TXT')):
            base, _ = os.path.splitext(file)
            while len(base) > 0 and base[-1].isdigit():
                base = base[:-1]

            if not base:
                raise ValueError('invalid drop item config file: %s' % file)

            filelist = glob.glob('%s/%s*.txt' % (dir, base))
            if len(filelist) == 0:
                raise ValueError('invalid drop item config file: %s' % file)

            if base not in monDict.keys():
                monDict[base] = filelist[0]

    for key, value in monDict.items():
        dropGenOneMonster(key, value)


def main():
    # convert mir3_config/MonItems/*.txt into dropitemconfig.inc
    # need to convert to utf8 encoding, MonItems/*.txt uses GB2312 encoding

    if len(sys.argv) != 2:
        raise ValueError('usage: python3 dropgen.py mir2_config/MonItems')

    printCode('// created by: %s %s' % (sys.argv[0], sys.argv[1]))
    printCode('// format:')
    printCode('//     {monsterName, itemName, group, probRecip, repeat, count}')
    printCode('')

    dropGen(sys.argv[1])


if __name__ == '__main__':
    main()
