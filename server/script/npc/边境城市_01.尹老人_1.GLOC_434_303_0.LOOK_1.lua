-- converted from Envir/Market_Def/04Potion_Kugkyung-01.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
        '欢迎光临。 请随便挑选。',
    },

    goods =
    {
        '金创药（小）',
        '魔法药（小）',
        '金创药（中）',
        '魔法药（中）',
        '金创药（大）',
        '魔法药（大）',
        '金创药（特）',
        '魔法药（特）',
        '太阳水',
    },

    trade  = {'恢复药水'},

    buyText =
    {
        '你想卖什么？',
    },

    tradeText =
    {
        '请把要出售的东西拿上来。',
    },
}
