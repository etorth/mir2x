import os
import re
import sys
import csv


def parse_mon(mon_dict):
    print('{   .name = u8"%s",' % mon_dict['name'])
    print('    .lookID = 0X%02X,' % mon_dict['appr'])

    if mon_dict['undead'] > 0:
        print('    .undead = 1,')

    if mon_dict['tameble'] > 0:
        print('    .tameable = 1,')

    print('    .view = %d,' % max(10, mon_dict['wz4']))

    if mon_dict['cooleye'] > 0:
        print('    .coolEye = 1,')

    print('    .deadFadeOut = 1,')

    if mon_dict['hp']  > 0:
        print('    .hp = %d,'  % mon_dict['hp'])

    if mon_dict['mp']  > 0:
        print('    .mp = %d,'  % mon_dict['mp'])

    if mon_dict['exp'] > 0:
        print('    .exp = %d,' % mon_dict['exp'])

    if mon_dict['dc'] > 0 or mon_dict['dcmax'] > 0:
        print('    .dc = {%d, %d},' % (mon_dict['dc'], mon_dict['dcmax']))

    if mon_dict['mc'] > 0 or mon_dict['mcmax'] > 0:
        print('    .mc = {%d, %d},' % (mon_dict['mc'], mon_dict['mcmax']))

    if mon_dict['wz2'] > 0 or mon_dict['wz3'] > 0:
        print('    .ac = {%d, %d},' % (mon_dict['wz2'], mon_dict['wz3']))

    if mon_dict['ac'] > 0:
        print('    .mac = {%d, %d},' % (mon_dict['ac'], mon_dict['ac']))

    if mon_dict['hit']    > 0:
        print('    .dcHit = %d,'  % mon_dict['hit'])

    if mon_dict['speed']  > 0:
        print('    .dcDodge = %d,'  % mon_dict['speed'])

    if mon_dict['firemac'] != 0 or mon_dict['icemac'] != 0 or mon_dict['lightmac'] != 0 or mon_dict['windmac'] != 0 or mon_dict['holymac'] != 0 or mon_dict['darkmac'] != 0 or mon_dict['phantommac'] != 0:
        print('    .acElem')
        print('    {')
        if mon_dict['firemac'   ] != 0: print('        .fire = %d,' % mon_dict['firemac'])
        if mon_dict['icemac'    ] != 0: print('        .ice = %d,' % mon_dict['icemac'])
        if mon_dict['lightmac'  ] != 0: print('        .light = %d,' % mon_dict['lightmac'])
        if mon_dict['windmac'   ] != 0: print('        .wind = %d,' % mon_dict['windmac'])
        if mon_dict['holymac'   ] != 0: print('        .holy = %d,' % mon_dict['holymac'])
        if mon_dict['darkmac'   ] != 0: print('        .dark = %d,' % mon_dict['darkmac'])
        if mon_dict['phantommac'] != 0: print('        .phantom = %d,' % mon_dict['phantommac'])
        print('    },')

    print('    .walkWait = %d,'  % max(500, mon_dict['walkwait']))

    if mon_dict['walkstep'] > 0:
        print('    .walkStep = %d,'  % mon_dict['walkstep'])

    print('    .attackWait = %d,'  % max(500, mon_dict['attack_spd']))

    print('},')
    print()


def parse_monster(filename):
    with open(filename, newline='') as csvfile:
        item_reader = csv.reader(csvfile)
        header = None
        mon_list = []
        for mon_row in item_reader:
            if not header:
                header = mon_row
                continue

            mon_dict = {}
            for i in range(len(header))[0:-2]:
                if i == 0: mon_dict[header[i].lower()] = mon_row[i]
                else     : mon_dict[header[i].lower()] = int(mon_row[i])

            mon_list.append(mon_dict)

        for mon_dict in sorted(mon_list, key = lambda mon_dict: (mon_dict['appr'], mon_dict['race'], mon_dict['raceimg'])):
            parse_mon(mon_dict)


# this script only generates references
# monsterrecord.inc copy result from this script, don't copy and paste
def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: python3 monstercsv2cpp.py mir2x/readme/sql2csv/King_Monster.csv")
    parse_monster(sys.argv[1])


if __name__ == "__main__":
    main()
