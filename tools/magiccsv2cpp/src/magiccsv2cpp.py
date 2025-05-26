import os
import re
import sys
import csv


def parse_magic(magic_dict):
    print('{   .name = u8"%s",' % magic_dict['magname'])
    if   magic_dict['mag_type'] == 0: print('    .elem = u8"火",')
    elif magic_dict['mag_type'] == 1: print('    .elem = u8"冰",')
    elif magic_dict['mag_type'] == 2: print('    .elem = u8"雷",')
    elif magic_dict['mag_type'] == 3: print('    .elem = u8"风",')
    elif magic_dict['mag_type'] == 4: print('    .elem = u8"神圣",')
    elif magic_dict['mag_type'] == 5: print('    .elem = u8"暗黑",')
    elif magic_dict['mag_type'] == 6: print('    .elem = u8"幻影",')
    else: pass

    print('    .req')
    print('    {')
    print('        .level = {%d, %d, %d},' % (magic_dict['needl1'], magic_dict['needl2'], magic_dict['needl3']))
    print('        .train = {%d, %d, %d},' % (magic_dict['l1train'], magic_dict['l2train'], magic_dict['l3train']))

    if   magic_dict['job'] == 0: print('        .job = u8"战士",')
    elif magic_dict['job'] == 1: print('        .job = u8"法师",')
    elif magic_dict['job'] == 2: print('        .job = u8"道士",')
    else: pass

    print('    },')

    if magic_dict['delay'] > 0:
        print('    .coolDown = %d,' % (magic_dict['delay'] * 10))

    if magic_dict['spell'] > 0:
        print('    .mp = %d,' % magic_dict['spell'])

    if magic_dict['defspell'] > 0:
        print('    .mpInc = %d,' % magic_dict['defspell'])

    if magic_dict['power'] > 0 or magic_dict['maxpower']:
        print('    .power = {%d, %d},' % (magic_dict['power'], magic_dict['maxpower']))

    if magic_dict['defpower'] > 0 or magic_dict['defmaxpower']:
        print('    .powerInc = {%d, %d},' % (magic_dict['defpower'], magic_dict['defmaxpower']))

    print('},')
    print()


def parse_magic_csv(filename):
    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        header = None
        mon_list = []
        for magic_row in item_reader:
            if not header:
                header = magic_row
                continue

            magic_dict = {}
            for i in range(len(header))[0:-2]:
                if i == 1: magic_dict[header[i].lower()] = magic_row[i]
                else     : magic_dict[header[i].lower()] = int(magic_row[i])

            mon_list.append(magic_dict)

        for magic_dict in mon_list:
            parse_magic(magic_dict)


# this script only generates references
# magicrecord.inc copy result from this script, don't copy and paste
def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 magiccsv2cpp.py mir2x/readme/sql2csv/King_Magic.csv")
    parse_magic_csv(sys.argv[1])


if __name__ == "__main__":
    main()
