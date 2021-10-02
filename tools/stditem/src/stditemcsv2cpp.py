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


def stditem_parse(filename):
    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        done_skip = False
        for item_row in item_reader:
            if not done_skip:
                done_skip = True
                continue

            item_name = item_row[1]
            item_type = num_2_type(int(item_row[2]))

            print('{   .name = u8"%s"' % item_name)
            print('    .type = u8"%s"' % item_type)
            print('    .weight = %s' % item_row[4])
            print('    .pkgGfxID = 0X%04X' % int(item_row[3]))

            # the csv has column "sac" but always zero
            # and looks the sdc is merged to mdc, need to check the stdmode to tell apart

            if item_type == '武器':
                print('    .equip')
                print('    {')
                print('        .duration = %d' % (int(item_row[11]) // 1000))
                print('        .dc = {%s, %s},' % (item_row[17], item_row[18]))
                print('        .ac = {%s, %s},' % (item_row[12], item_row[13]))
                print('        .mdc = {%s, %s},' % (item_row[22], item_row[23]))
                print('        .mac = {%s, %s},' % (item_row[15], item_row[16]))
                print('        .sdc = {%s, %s},' % (item_row[19], item_row[20]))
                print('    },')

            print('},')
            print()


def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 stditemcsv2cpp.py <csv-file-name>")
    stditem_parse(sys.argv[1])


if __name__ == "__main__":
    main()
