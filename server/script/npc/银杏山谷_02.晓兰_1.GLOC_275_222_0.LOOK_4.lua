-- converted from Envir/Market_Def/08Accessory_Eunhang-02.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '饰品',

    greet =
    {
    },

    goods =
    {
        '六绝星环',
        '玄铁指环',
        '蛇眼戒指',
        '黑檀手镯',
        '黑檀项链',
        '琥珀项链',
    },

    trade  = {'戒指', '手镯', '项链'},
    repair = {'戒指', '手镯', '项链'},

    buyText =
    {
        '你想买饰品?',
    },

    tradeText =
    {
        '你想出售饰品？',
    },

    repairText =
    {
        '你想修理饰品？',
    },
}
