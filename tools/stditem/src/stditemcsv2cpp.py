import os
import re
import sys
import csv

def get_item_type(item_dict):
    if item_dict['stdmode'] == 0:
        return '恢复药水'
    elif item_dict['stdmode'] in [1, 40]:
        return '肉'
    elif item_dict['stdmode'] == 2:
        return '道具'
    elif item_dict['stdmode'] == 3:
        if item_dict['shape'] in [1, 2, 3, 5]:
            return '传送卷轴'
        elif item_dict['shape'] in [4, 9, 10]:
            return '功能药水'
        elif item_dict['shape'] == 11:
            return '道具'
        elif item_dict['shape'] == 12:
            return '强效药水'
        else:
            return '道具'
    elif item_dict['stdmode'] == 4:
        return '技能书'
    elif item_dict['stdmode'] in [5, 6]:
        return '武器'
    elif item_dict['stdmode'] in [10, 11]:
        return '衣服'
    elif item_dict['stdmode'] == 15:
        return '头盔'
    elif item_dict['stdmode'] in [19, 20, 21]:
        return '项链'
    elif item_dict['stdmode'] in [22, 23]:
        return '戒指'
    elif item_dict['stdmode'] in [24, 26]:
        return '手镯'
    elif item_dict['stdmode'] == 25:
        if item_dict['shape'] in [1, 2]:
            return '药粉'
        elif item_dict['shape'] in [5, 7]:
            return '护身符'
        else:
            return '道具'
    elif item_dict['stdmode'] == 30:
        if re.search('勋章', item_dict['name']):
            return '勋章'
        else:
            return '火把'
    elif item_dict['stdmode'] == 31:
        return '道具' # 打捆物品
    elif item_dict['stdmode'] == 41:
        return '金币'
    elif item_dict['stdmode'] == 43:
        return '矿石'
    elif item_dict['stdmode'] == 44:
        return '道具'
    elif item_dict['stdmode'] == 45:
        return '骰子'
    elif item_dict['stdmode'] == 46:
        return '道具'
    elif item_dict['stdmode'] == 47:
        return '道具'
    elif item_dict['stdmode'] == 50:
        return '兑换券'
    return '道具'


def parse_book(item_dict):
    print('    .book')
    print('    {')

    if   item_dict['shape'] == 0: print('        .job = u8"战士",')
    elif item_dict['shape'] == 1: print('        .job = u8"法师",')
    elif item_dict['shape'] == 2: print('        .job = u8"道士",')

    print('        .level = %d,' % item_dict['duramax'])
    print('    },')


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

def parse_dope(item_dict):
    print('    .dope')
    print('    {')

    if item_dict['ac'] > 0:
        print('        .hp = %d,' % item_dict['ac'])

    if item_dict['mac'] > 0:
        print('        .mp = %d,' % item_dict['mac'])

    if item_dict['dc'] > 0:
        print('        .dc = %d,' % item_dict['dc'])

    if item_dict['mc'] > 0:
        print('        .mdc = %d,' % item_dict['mc'])

    if item_dict['sac'] > 0:
        print('        .sdc = %d,' % item_dict['sac'])

    if item_dict['ac2'] > 0:
        print('        .speed = %d,' % item_dict['ac2'])

    if item_dict['mac2'] > 0:
        print('        .time = %d,' % item_dict['mac2'])

    print('    },')


def parse_equip_require(item_dict):
    if item_dict['needlevel'] > 0:
        print('        .req')
        print('        {')
        if   item_dict['need'] == 0: print('            .level = %d,' % item_dict['needlevel'])
        elif item_dict['need'] == 1: print('            .dc = %d,'    % item_dict['needlevel'])
        elif item_dict['need'] == 2: print('            .mdc = %d,'   % item_dict['needlevel'])
        elif item_dict['need'] == 3: print('            .sdc = %d,'   % item_dict['needlevel'])
        else: pass
        print('        },')


def parse_necklace(item_dict):
    print('    .equip')
    print('    {')
    print('        .duration = %d,' % (item_dict['duramax'] // 1000))

    hit = 0
    dodge = 0
    speed = 0
    hprecover = 0
    mprecover = 0
    mdcdodge = 0
    luck = 0

    if item_dict['stdmode'] == 19:
        mdcdodge += item_dict['ac2']
        luck += item_dict['mac']
        luck -= item_dict['mac2']
    elif item_dict['stdmode'] == 20:
        hit += item_dict['ac2']
        dodge += item_dict['mac2']
    elif item_dict['stdmode'] == 21:
        speed += item_dict['ac']
        speed -= item_dict['mac']
        hprecover += item_dict['ac2']
        mprecover += item_dict['mac2']

    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        print('        .dc = {%d, %d},' % (item_dict['dc'], item_dict['dc2']))

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            print('        .mdc = {%d, %d},' % (item_dict['mc'], item_dict['mc2']))
        elif item_dict['mc_type'] == 2:
            print('        .sdc = {%d, %d},' % (item_dict['mc'], item_dict['mc2']))

    if hit > 0:
        print('        .hit = %d,' % hit)

    if dodge > 0:
        print('        .dodge = %d,' % dodge)

    if speed > 0:
        print('        .speed = %d,' % speed)

    if hprecover > 0:
        print('        .hprecover = %d,' % hprecover)

    if mprecover > 0:
        print('        .mprecover = %d,' % mprecover)

    if mdcdodge > 0:
        print('        .mdcdodge = %d,' % mdcdodge)

    if luck != 0:
        print('        .luck = %d,' % luck)

    parse_equip_require(item_dict)
    print('    },')


def parse_equip(item_dict):
    item_type = get_item_type(item_dict)

    print('    .equip')
    print('    {')
    print('        .duration = %d,' % (item_dict['duramax'] // 1000))

    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        print('        .dc = {%d, %d},' % (item_dict['dc'], item_dict['dc2']))

    if (item_dict['ac'] > 0 or item_dict['ac2'] > 0) and item_type != '项链':
        print('        .ac = {%d, %d},' % (item_dict['ac'], item_dict['ac2']))

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] == 1:
            print('        .mdc = {%d, %d},' % (item_dict['mc'], item_dict['mc2']))
        elif item_dict['mc_type'] == 2:
            print('        .sdc = {%d, %d},' % (item_dict['mc'], item_dict['mc2']))

    if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
        print('        .mac = {%d, %d},' % (item_dict['mac'], item_dict['mac2']))

    parse_equip_require(item_dict)
    print('    },')


def parse_item(item_dict):
    item_name = item_dict['name']
    item_type = get_item_type(item_dict)

    print('{   .name = u8"%s"' % item_name)
    print('    .type = u8"%s"' % item_type)
    print('    .weight = %d' % item_dict['weight'])
    print('    .pkgGfxID = 0X%04X' % item_dict['looks'])

    if item_type == '恢复药水':
        parse_potion(item_dict)
    elif item_type == '功能药水':
        pass
    elif item_type == '强效药水':
        parse_dope(item_dict)
    elif item_type == '技能书':
        parse_book(item_dict)
    elif item_type == '项链':
        parse_necklace(item_dict)
    elif item_type in ['头盔', '戒指', '武器', '衣服', '手镯']:
        parse_equip(item_dict)

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
