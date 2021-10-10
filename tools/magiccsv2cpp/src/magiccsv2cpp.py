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

    print('    .icon = 0X%08X,' % (magic_dict['magid'] + 0X05001000 - 1))
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
