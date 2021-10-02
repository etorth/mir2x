import os
import sys
import csv

def num_2_type(num):
    if num ==  0: return '恢复药水'
    if num ==  1: return '肉'
    if num ==  2: return '包子'
    if num ==  3: return '功能药水'
    if num ==  4: return '技能书'
    if num ==  5: return '武器'
    if num ==  6: return '高级武器'
    if num == 10: return '衣服（男）'
    if num == 10: return '衣服（女）'
    if num == 15: return '头盔'
    if num == 19: return '项链'
    if num == 20: return '项链'
    if num == 21: return '项链'
    if num == 22: return '戒指'
    if num == 23: return '戒指'
    if num == 24: return '手镯'
    if num == 25: return '药粉|护身符'
    if num == 26: return '手镯'
    if num == 30: return '火把|勋章'
    if num == 31: return '恢复药水'
    if num == 40: return '肉'
    if num == 41: return '金币'
    if num == 43: return '矿石'
    if num == 44: return '道具'
    return '道具'


def parse_potion(item_dict):
    print('    .potion')
    print('    {')

    if item_dict['ac'] > 0:
        print('        .hp = %d,' % item_dict['ac'])

    if item_dict['mac'] > 0:
        print('        .mp = %d,' % item_dict['mac'])

    if item_dict['shape'] == 0:
        print('        .time = 1,')
    elif item_dict['shape'] == 1:
        pass # add HP/MP immediately
    else:
        pass # don't know what kind of effect this potion can help

    print('    },')


def parse_item(item_dict):
    item_name = item_dict['name']
    item_type = num_2_type(item_dict['stdmode'])

    print('{   .name = u8"%s"' % item_name)
    print('    .type = u8"%s"' % item_type)
    print('    .weight = %d' % item_dict['weight'])
    print('    .pkgGfxID = 0X%04X' % item_dict['looks'])

    if item_type == '恢复药水':
        parse_potion(item_dict)

    elif item_type == '武器':
        print('    .equip')
        print('    {')
        print('        .duration = %d,' % (int(item_dict['duramax']) // 1000))
        if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
            print('        .dc = {%d, %d},' % (item_dict['dc'], item_dict['dc2']))

        if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
            print('        .ac = {%d, %d},' % (item_dict['ac'], item_dict['ac2']))

        if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
            if item_dict['mc_type'] == 1:
                if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
                    print('        .mdc = {%d, %d},' % (item_dict['mc'], item_dict['mc2']))
            elif item_dict['mc_type'] == 2:
                if item_dict['sac'] > 0 or item_dict['sac'] > 0:
                    print('        .sdc = {%d, %d},' % (item_dict['sac'], item_dict['sac2']))

        if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
            print('        .mac = {%d, %d},' % (item_dict['mac'], item_dict['mac2']))

        print('    },')

        if item_dict['needlevel'] > 0:
            print('    .req')
            print('    {')
            if   item_dict['need'] == 0: print('        .level = %d,' % item_dict['needlevel'])
            elif item_dict['need'] == 1: print('        .dc = %d,'    % item_dict['needlevel'])
            elif item_dict['need'] == 2: print('        .mdc = %d,'   % item_dict['needlevel'])
            elif item_dict['need'] == 3: print('        .sdc = %d,'   % item_dict['needlevel'])
            else: pass
            print('    },')

    print('},')
    print()


def parse_stditem(filename):
    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        header = None
        for item_row in item_reader:
            if not header:
                header = item_row
                continue

            item_dict = {}
            for i in range(len(header))[0:-1]:
                if   i == 0: pass
                elif i == 1: item_dict[header[i].lower()] = item_row[i]
                else       : item_dict[header[i].lower()] = int(item_row[i])

            parse_item(item_dict)


def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 stditemcsv2cpp.py <csv-file-name>")
    parse_stditem(sys.argv[1])


if __name__ == "__main__":
    main()
