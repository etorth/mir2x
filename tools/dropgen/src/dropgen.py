import os
import re
import sys
import glob

# global re pattern
# compile it as use all times
g_item = re.compile('^\s*(\d+)/(\d+)\s+(\S+)(\s+(\d+))?\s*$')
#                     --- --- - ---     ---      ---
#                      ^   ^  ^  ^       ^        ^
#                      |   |  |  |       |        |
#                      |   |  |  |       |        +---- item count
#                      |   |  |  |       +------------- item name
#                      |   +--+--+--------------------- a / b
#                      +------------------------------- optional comment markers

def printCode(s):
    print('--->%s' % s)


def dropGenOneMonster(name, file):
    codeMap = {}
    with open(file, 'r', encoding='gb2312') as fp:
        for line in fp:
            match = g_item.search(line.strip())
            if match:
                probNum = int(match.group(1))
                probDen = int(match.group(2))

                itemName = match.group(3)
                if match.group(5):
                    itemCount = int(match.group(5))
                else:
                    itemCount = 1

                probRecip = max(1, round(probDen / probNum))
                if itemName == '金币' or itemName == '钱币':
                    printCode('{u8"%s", u8"金币（小）", 0, %d, 1, %s},' % (name, probRecip, itemCount)) # gold in total
                else:
                    codeLine = '{u8"%s", u8"%s", 0, %d, ' % (name, itemName, probRecip)
                    if codeLine not in codeMap.keys():
                        codeMap[codeLine] = itemCount
                    else:
                        codeMap[codeLine] = codeMap[codeLine] + itemCount
            else:
                # doesn't match anything
                # print the skipped lines for double check
                print('[WARN] skipped: %s' % line.strip())

    for code, count in codeMap.items():
        printCode('%s%d, 1},' % (code, count))


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
    printCode('//')
    printCode('// always use 金币（小）to represent gold')
    printCode('// code will figure out 小中大 based on SDItem::count')
    printCode('')

    dropGen(sys.argv[1])


if __name__ == '__main__':
    main()
