-- converted from Envir/Market_Def/08Accessory_HalfNight-8.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '饰品',

    greet =
    {
    },

    goods =
    {
        '骷髅戒指',
        '小手镯',
        '蓝翡翠项链',
        '珍珠戒指',
        '道士手镯',
        '蛇眼戒指',
        '黑檀手镯',
    },

    trade  = {'戒指', '手镯', '项链'},
    repair = {'戒指', '手镯', '项链'},

    buyText =
    {
        '你想买饰品？买什么？仔细挑选一下吧。',
    },

    tradeText =
    {
        '请把不用的饰品卖给我。',
    },

    repairText =
    {
        '那我给你修吧。你要修什么？',
    },
}
