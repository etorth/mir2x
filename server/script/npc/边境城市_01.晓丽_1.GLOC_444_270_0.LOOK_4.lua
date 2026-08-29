-- converted from Envir/Market_Def/08Accessory_Kugkyung-01.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '饰品',

    greet =
    {
    },

    goods =
    {
        '古铜戒指',
        '黑色水晶戒指',
        '骷髅戒指',
        '小手镯',
        '黑色水晶项链',
        '魔鬼项链',
        '蓝翡翠项链',
    },

    trade  = {'戒指', '手镯', '项链'},
    repair = {'戒指', '手镯', '项链'},

    buyText =
    {
        '你想买饰品？',
    },

    tradeText =
    {
        '你想出售饰品？',
    },

    repairText =
    {
        '你想修理饰品?',
    },
}
