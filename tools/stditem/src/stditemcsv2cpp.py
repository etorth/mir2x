import os
import re
import sys
import csv

def skip_item_index(item_index):
    if   item_index ==   0: return True # 金币
    elif item_index == 201: return True # 韩服（男）
    elif item_index == 202: return True # 韩服（女）
    elif item_index == 289: return True # "_水饺200",
    elif item_index == 290: return True # "_水饺400",
    elif item_index == 291: return True # "_水饺600",
    elif item_index == 292: return True # "_水饺800",
    elif item_index == 293: return True # "_水饺1000",
    elif item_index == 343: return True # 韩服（男）
    elif item_index == 381: return True # 七点白蛇血, dup 1
    elif item_index == 368: return True # 汤药
    elif item_index == 383: return True # 沃玛角
    elif item_index == 428: return True # 七点白蛇血, dup 2
    elif item_index == 519: return True # 沃玛角
    elif item_index == 580: return True # 千年毒蛇牙齿
    elif item_index == 589: return True # _丸药（500）
    elif item_index == 590: return True # _丸药（2000）
    elif item_index == 606: return True # _丸药（1000）
    elif item_index == 607: return True # _丸药（5000）
    elif item_index == 608: return True # _丸药（10000）
    elif item_index == 609: return True # _丸药（20000）
    elif item_index == 611: return True # _丸药（50000）
    elif item_index == 630: return True # 添加手套4
    elif item_index == 660: return True # 法师剑1
    elif item_index == 663: return True # 法师剑2
    elif item_index == 666: return True # 法师剑4
    elif item_index == 671: return True # 添加头盔3
    elif item_index == 672: return True # 添加头盔4
    elif item_index == 674: return True # 添加头盔5
    elif item_index == 784: return True # 阿才的书
    return False


def get_item_rename(item_dict):
    if   item_dict['idx'] ==   66 : return '沃玛号角_0'         # <- 沃玛号角
    elif item_dict['idx'] ==   84 : return '小手镯_0'           # <- 小手镯
    elif item_dict['idx'] ==  119 : return '小手镯_1'           # <- 小手镯, same name but with different gfx, and different attributes
    elif item_dict['idx'] ==  379 : return '沃玛号角_1'         # <- 沃玛勇士号角
    elif item_dict['idx'] ==  557 : return '栗子_0'             # <- 栗子2
    elif item_dict['idx'] ==  558 : return '栗子_1'             # <- 栗子2
    elif item_dict['idx'] ==  559 : return '栗子_2'             # <- 栗子3
    elif item_dict['idx'] ==  560 : return '栗子_3'             # <- 栗子4
    elif item_dict['idx'] ==  561 : return '栗子_4'             # <- 栗子5
    elif item_dict['idx'] ==  562 : return '栗子_5'             # <- 栗子6
    elif item_dict['idx'] ==  563 : return '栗子_6'             # <- 栗子7
    elif item_dict['idx'] ==  564 : return '栗子_7'             # <- 栗子8
    elif item_dict['idx'] ==  565 : return '栗子_8'             # <- 栗子9
    elif item_dict['idx'] ==  566 : return '栗子_9'             # <- 栗子10
    elif item_dict['idx'] ==  588 : return '丸药'               # <- _丸药（100）
    elif item_dict['idx'] ==  629 : return '铁金手套'           # <- 添加手套2
    elif item_dict['idx'] ==  805 : return '暗黑之药水'         # <- 汤药
    elif item_dict['idx'] == 1053 : return '绝世极品战甲（男）' # <- 绝世极品战甲（男）1
    elif item_dict['idx'] == 1054 : return '绝世极品战甲（女）' # <- 绝世极品战甲（女）1

    if item_dict['stdmode'] == 4 and re.search('（秘籍）', item_dict['name']):
        return item_dict['name'][0:-4]

    return item_dict['name']

def get_item_description(item_rename):
    # item_rename is unique
    # so we can use it solely as the item key
    if   item_rename == '云龙手镯': return '李云龙的传家宝。'
    elif item_rename == '屠龙'    : return '用天外玄铁打造，刀身刻有龙纹，潜藏着强大的气场，是荣耀和力量的象征。'
    elif item_rename == '草鞋'    : return '用香蒲编成的凉鞋，价格低廉，是老百姓常穿的鞋子。'
    elif item_rename == '皮靴'    : return '用皮革做成的鞋子，坚固耐用，是老百姓常穿的鞋子。'
    elif item_rename == '布鞋'    : return '由布做的鞋，柔软舒适，是老百姓常穿的鞋子。'
    elif item_rename == '褐色栗子': return '营养丰富的褐色栗子，益气补脾，健胃厚肠。'
    elif item_rename == '金色栗子': return '营养丰富的金色栗子，益气补脾，健胃厚肠，是栗子中的极品。'
    elif item_rename == '银色栗子': return '营养丰富的金色栗子，益气补脾，健胃厚肠，是栗子中的珍品。'
    elif item_rename == '铜色栗子': return '营养丰富的金色栗子，益气补脾，健胃厚肠，是栗子中的上品。'
    return None


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
    elif item_dict['stdmode'] == 53:
        return '鞋'
    elif item_dict['stdmode'] == 54:
        return '标枪'
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
        pass # unknown types

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
        print('        .mc = %d,' % item_dict['mc'])

    if item_dict['sac'] > 0:
        print('        .sc = %d,' % item_dict['sac'])

    if item_dict['ac2'] > 0:
        print('        .speed = %d,' % item_dict['ac2'])

    if item_dict['mac2'] > 0:
        print('        .time = %d,' % item_dict['mac2'])

    print('    },')


def parse_equip_require(item_dict):
    if item_dict['needlevel'] > 0:
        print('        .req')
        print('        {')
        if   item_dict['need'] == 0: print('            .level = %d,' % min(60, item_dict['needlevel']))
        elif item_dict['need'] == 1: print('            .dc = %d,' % item_dict['needlevel'])
        elif item_dict['need'] == 2: print('            .mc = %d,' % item_dict['needlevel'])
        elif item_dict['need'] == 3: print('            .sc = %d,' % item_dict['needlevel'])
        else: pass
        print('        },')


def parse_equip_attr_helmet(item_dict):
    # only helmet has non-zero sac field, which means acElem, not parsed here
    # reset fileds are parsed as cloth
    return parse_equip_attr_cloth(item_dict)


def parse_equip_attr_necklace(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] in [0, 1]:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]

        if item_dict['mc_type'] in [0, 2]:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['stdmode'] == 19:
        if item_dict['ac2'] > 0:
            item_attr['mcDodge'] = item_dict['ac2']
        if item_dict['mac2'] > 0:
            item_attr['luckCurse'] = item_dict['mac2']

    elif item_dict['stdmode'] == 20:
        if item_dict['ac2'] > 0:
            item_attr['dcHit'] = item_dict['ac2']

        # for 神勇之物 mac is 1, but seems game doesn't use it
        # all others of necklace has mac as zero

        # also for 神勇之物 mac2 is 1 but looks the game take its dcDodge as 0
        # this is strange, but we still take it as dcDodge

        # and for dc, 神勇之物 is 7 but in game it's only 1
        # ok I think 神勇之物 is not using this row in the csv database

        if item_dict['mac2'] > 0:
            item_attr['dcDodge'] = item_dict['mac2']

    elif item_dict['stdmode'] == 21:
        if item_dict['ac'] > 0:
            item_attr['speed'] = item_dict['ac']

        if item_dict['ac2'] > 0:
            item_attr['hpRecover'] = item_dict['ac2']

        if item_dict['mac2'] > 0:
            item_attr['mpRecover'] = item_dict['mac2']

    return item_attr


def parse_equip_attr_armring(item_dict):
    item_attr = {}
    if item_dict['stdmode'] == 24:
        if item_dict['ac2'] > 0:
            item_attr['dcHit'] = item_dict['ac2']

        if item_dict['mac2'] > 0:
            item_attr['dcDodge'] = item_dict['mac2']

    elif item_dict['stdmode'] == 26:
        if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
            item_attr['ac'] = [item_dict['ac'], item_dict['ac2']]

        if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
            item_attr['mac'] = [item_dict['mac'], item_dict['mac2']]
    else:
        pass

    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] in [0, 1]:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]

        if item_dict['mc_type'] in [0, 2]:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    return item_attr


def parse_equip_attr_ring(item_dict):
    item_attr = {}
    if item_dict['stdmode'] == 22:
        # explain all fields as cloth does
        # means column name means what it literlly means
        return parse_equip_attr_cloth(item_dict)

    elif item_dict['stdmode'] == 23:
        if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
            item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

        if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
            if item_dict['mc_type'] in [0, 1]:
                item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]

            if item_dict['mc_type'] in [0, 2]:
                item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]
        if item_dict['ac'] > 0:
            item_attr['speed'] = item_dict['ac']

        # TODO not sure if I should ignore ac2
        # because actually for those rings with nonzer ac2, the game doesn't support

        if item_dict['ac2'] > 0:
            item_attr['dcHit'] = item_dict['ac2']

        # TODO not sure if I should ignore mac2
        # because actually for those rings with nonzer mac2, the game doesn't support

        if item_dict['mac2'] > 0:
            item_attr['dcDodge'] = item_dict['mac2']

    else:
        pass

    return item_attr


def parse_equip_attr_medal(item_dict):
    return parse_equip_attr_cloth(item_dict)


def parse_equip_attr_cloth(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] in [0, 1]:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]

        if item_dict['mc_type'] in [0, 2]:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['ac'] > 0 or item_dict['ac2'] > 0:
        item_attr['ac'] = [item_dict['ac'], item_dict['ac2']]

    if item_dict['mac'] > 0 or item_dict['mac2'] > 0:
        item_attr['mac'] = [item_dict['mac'], item_dict['mac2']]

    return item_attr


def parse_equip_attr_boot(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0:
        item_attr['comfort'] = item_dict['dc']

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        item_attr['load'] = {}

        if item_dict['mc'] > 0:
            item_attr['load']['body'] = item_dict['mc']

        if item_dict['mc2'] > 0:
            item_attr['load']['hand'] = item_dict['mc2']

    return item_attr


def parse_equip_attr_weapon(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['mc'] > 0 or item_dict['mc2'] > 0:
        if item_dict['mc_type'] in [0, 1]:
            item_attr['mc'] = [item_dict['mc'], item_dict['mc2']]

        if item_dict['mc_type'] in [0, 2]:
            item_attr['sc'] = [item_dict['mc'], item_dict['mc2']]

    if item_dict['stdmode'] == 5:
        if item_dict['ac2'] > 0:
            item_attr['dcHit'] = item_dict['ac2']

        if item_dict['mac2'] > 10:
            item_attr['speed'] = item_dict['mac2'] - 10
        elif item_dict['mac2'] > 0:
            pass # only 寂幻之刃 has value 3, but looks it doesn't show in the game, looks ignored


    elif item_dict['stdmode'] == 6:
        if item_dict['ac2'] > 100:
            item_attr['dcHit'] = 100 - item_dict['ac2'] # negative, only 风之鹤嘴锄 has this negative attribute
        elif item_dict['ac2'] > 0:
            item_attr['dcHit'] = item_dict['ac2']

        if item_dict['mac2'] > 10:
            item_attr['speed'] = item_dict['mac2'] - 10
        elif item_dict['mac2'] > 0:
            item_attr['speed'] = -item_dict['mac2']

    else:
        pass

    return item_attr


def parse_equip_attr_javelin(item_dict):
    item_attr = {}
    if item_dict['dc'] > 0 or item_dict['dc2'] > 0:
        item_attr['dc'] = [item_dict['dc'], item_dict['dc2']]

    if item_dict['ac2'] > 0:
        item_attr['dcHit'] = item_dict['ac2']

    return item_attr


def parse_equip(item_dict):
    item_type = get_item_type(item_dict)
    item_attr = {}

    print('    .equip')
    print('    {')
    print('        .duration = %d,' % max(1, item_dict['duramax'] // 1000))

    if   item_type == '头盔': item_attr = parse_equip_attr_helmet  (item_dict)
    elif item_type == '项链': item_attr = parse_equip_attr_necklace(item_dict)
    elif item_type == '手镯': item_attr = parse_equip_attr_armring (item_dict)
    elif item_type == '戒指': item_attr = parse_equip_attr_ring    (item_dict)
    elif item_type == '勋章': item_attr = parse_equip_attr_medal   (item_dict)
    elif item_type == '衣服': item_attr = parse_equip_attr_cloth   (item_dict)
    elif item_type == '鞋'  : item_attr = parse_equip_attr_boot    (item_dict)
    elif item_type == '武器': item_attr = parse_equip_attr_weapon  (item_dict)
    elif item_type == '标枪': item_attr = parse_equip_attr_javelin (item_dict)
    else: raise ValueError(item_dict['name'], item_type)

    if 'dc' in item_attr:
        print('        .dc = {%d, %d},' % (item_attr['dc'][0], item_attr['dc'][1]))

    if 'mc' in item_attr:
        print('        .mc = {%d, %d},' % (item_attr['mc'][0], item_attr['mc'][1]))

    if 'sc' in item_attr:
        print('        .sc = {%d, %d},' % (item_attr['sc'][0], item_attr['sc'][1]))

    if 'ac' in item_attr:
        print('        .ac = {%d, %d},' % (item_attr['ac'][0], item_attr['ac'][1]))

    if 'mac' in item_attr:
        print('        .mac = {%d, %d},' % (item_attr['mac'][0], item_attr['mac'][1]))

    if 'dcHit' in item_attr:
        print('        .dcHit = %d,' % item_attr['dcHit'])

    if 'mcHit' in item_attr:
        print('        .mcHit = %d,' % item_attr['mcHit'])

    if 'dcDodge' in item_attr:
        print('        .dcDodge = %d,' % item_attr['dcDodge'])

    if 'mcDodge' in item_attr:
        print('        .mcDodge = %d,' % item_attr['mcDodge'])

    if 'speed' in item_attr:
        print('        .speed = %d,' % item_attr['speed'])

    if 'comfort' in item_attr:
        print('        .comfort = %d,' % item_attr['comfort'])

    if 'hpRecover' in item_attr:
        print('        .hpRecover = %d,' % item_attr['hpRecover'])

    if 'mpRecover' in item_attr:
        print('        .mpRecover = %d,' % item_attr['mpRecover'])

    if 'luckCurse' in item_attr:
        print('        .luckCurse = %d,' % item_attr['luckCurse'])

    if item_dict['func_type'] > 0:
        print('        .dcElem')
        print('        {')

        if   item_dict['func_type'] == 1: print('            .fire = %d,'    % item_dict['func'])
        elif item_dict['func_type'] == 2: print('            .ice = %d,'     % item_dict['func'])
        elif item_dict['func_type'] == 3: print('            .light = %d,'   % item_dict['func'])
        elif item_dict['func_type'] == 4: print('            .wind = %d,'    % item_dict['func'])
        elif item_dict['func_type'] == 5: print('            .holy = %d,'    % item_dict['func'])
        elif item_dict['func_type'] == 6: print('            .dark = %d,'    % item_dict['func'])
        elif item_dict['func_type'] == 7: print('            .phantom = %d,' % item_dict['func'])
        else                            : pass

        print('        },')

    # seems all sac2 are zeros
    # only 头盔 has sac non-zero, this means only 头盔 can add acElem?

    if item_type == '头盔' and item_dict['sac'] > 0:
        print('        .acElem')
        print('        {')

        if   item_dict['sac'] == 1: print('            .fire = 1,')
        elif item_dict['sac'] == 2: print('            .ice = 1,')
        elif item_dict['sac'] == 3: print('            .light = 1,')
        elif item_dict['sac'] == 4: print('            .wind = 1,')
        elif item_dict['sac'] == 5: print('            .holy = 1,')
        elif item_dict['sac'] == 6: print('            .dark = 1,')
        elif item_dict['sac'] == 7: print('            .phantom = 1,')
        else                      : pass

        print('        },')

    if 'load' in item_attr:
        print('        .load')
        print('        {')

        if 'hand' in item_attr['load']:
            print('            .hand = %d,' % item_attr['load']['hand'])

        if 'body' in item_attr['load']:
            print('            .body = %d,' % item_attr['load']['body'])

        if 'inventory' in item_attr['load']:
            print('            .inventory = %d,' % item_attr['load']['inventory'])

        print('        },')

    parse_equip_require(item_dict)
    print('    },')


def parse_item(item_dict):
    item_type = get_item_type(item_dict)
    print('{   .name = u8"%s",' % item_dict['rename'])
    print('    .type = u8"%s",' % item_type)
    print('    .weight = %d,' % max(1, item_dict['weight']))
    print('    .pkgGfxID = 0X%04X,' % item_dict['looks'])

    if item_type == '头盔':
        if item_dict['charlooks'] > 0:
            print('    .shape = %d,' % item_dict['charlooks'])
        else:
            print('    .shape = 0, // TODO this helmet has no associated gfx resource')

    elif item_type == '衣服':
        if re.search('布衣', item_dict['name']):
            print('    .shape = 1,') # this database changes 1 to all fancy cloths
        elif item_dict['shape'] == 1:
            print('    .shape = 0, // TODO this cloth has no associated gfx resource')
        else:
            print('    .shape = %d,' % item_dict['shape'])

    elif item_type == '武器':
        # looks all weapons have non-zero number associted to 'shape' field
        # but some numbers are not mapped to the exact weapon, i.e. 飞龙剑, it uses shape = 14, which is also used by 井中月 and 生死宝刀
        print('    .shape = %d,' % item_dict['shape'])

    if item_type == '恢复药水':
        parse_potion(item_dict)
    elif item_type == '功能药水':
        pass
    elif item_type == '强效药水':
        parse_dope(item_dict)
    elif item_type == '技能书':
        parse_book(item_dict)
    elif item_type in ['头盔', '项链', '手镯', '戒指', '勋章', '衣服', '武器', '鞋', '标枪']:
        parse_equip(item_dict)

    item_desp = get_item_description(item_dict['rename'])
    if item_desp:
        print('    .description = u8"%s",' % item_desp)

    print('},')
    print()


def parse_stditem(filename):
    # print all special items here
    # need to simulaneously update the variable ``found_items" to detect duplications
    print(
'''// auto generated by: tools/stditem/src/stditemcsv2cpp.py mir2x/readme/sql2csv/King_StdItems.csv
// for reference: http://db.178.com/mir3/item-list

{},

// put all hand-made items here
// edits here invalidate game server database

{   .name = u8"金币（小）",
    .type = u8"金币",
    .pkgGfxID = 0X0078,
},

{   .name = u8"金币（中）",
    .type = u8"金币",
    .pkgGfxID = 0X0079,
},

{   .name = u8"金币（大）",
    .type = u8"金币",
    .pkgGfxID = 0X007A,
},

{   .name = u8"金币（特）",
    .type = u8"金币",
    .pkgGfxID = 0X007B,
},

{   .name = u8"金币（超）",
    .type = u8"金币",
    .pkgGfxID = 0X007C,
},

// below this line all items are from King_StdItems.csv
// item has been sorted by pkgGfxID, not using original order in King_StdItems.csv
''')

    found_items = set()
    found_items.add('金币（小）')
    found_items.add('金币（中）')
    found_items.add('金币（大）')
    found_items.add('金币（特）')
    found_items.add('金币（超）')

    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        header = None
        item_list = []
        for item_row in item_reader:
            if not header:
                header = item_row
                continue

            if skip_item_index(int(item_row[0])):
                continue

            item_dict = {}
            for i in range(len(header))[0:-1]:
                if   i == 0: item_dict['idx'] = int(item_row[i]) # csv has special leading bytes, it's '\ufeffIdx'
                elif i == 1: item_dict[header[i].lower()] = item_row[i]
                else       : item_dict[header[i].lower()] = int(item_row[i])

            # 秘籍 has attributes for job/level
            # skip plain book like 基本剑术, and rename 基本剑术（秘籍）to 基本剑术

            if item_dict['stdmode'] == 51:
                continue

            # some wired 秘籍 created by the private servers?
            # item has name like: 聚集灵魂火符（秘籍）, 分散灵魂火符（秘籍）, 幽灵盾（雷）（秘籍）, 幽灵盾（风）（秘籍）
            if item_dict['stdmode'] == 99 and re.search('（秘籍）', item_dict['name']):
                continue

            item_dict['rename'] = get_item_rename(item_dict)
            item_list.append(item_dict)

        for item_dict in sorted(item_list, key = lambda item_dict: (item_dict['looks'], item_dict['stdmode'], item_dict['shape'], item_dict['idx'])):
            if item_dict['rename'] in found_items:
                raise ValueError("found duplicates: index = %d, name = %s" % (item_dict['idx'], item_dict['rename']))

            parse_item(item_dict)
            found_items.add(item_dict['rename'])


def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 stditemcsv2cpp.py mir2x/readme/sql2csv/King_StdItems.csv")
    parse_stditem(sys.argv[1])


if __name__ == "__main__":
    main()
