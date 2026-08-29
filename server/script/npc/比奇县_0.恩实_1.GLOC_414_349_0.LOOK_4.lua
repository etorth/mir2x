-- converted from Envir/Market_Def/08Accessory_Bichon-0.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '饰品',

    greet =
    {
        '欢迎光临，你需要什么？',
    },

    goods =
    {
        '指环',
        '牛角戒指',
        '蓝色水晶戒指',
        '铁手镯',
        '小手镯',
        '银手镯',
        '钢手镯',
        '大手镯',
        '金项链',
        '传统项链',
        '灯笼项链',
        '白色虎齿项链',
        '皮制手套',
    },

    trade  = {'戒指', '手镯', '项链'},
    repair = {'戒指', '手镯', '项链'},

    buyText =
    {
        '你想买饰品?',
    },

    tradeText =
    {
        '你想出售饰品?',
    },

    repairText =
    {
        '你想修理饰品?',
    },
}
