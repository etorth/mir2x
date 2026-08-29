-- converted from Envir/Market_Def/04Potion_Mongchon-74.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
    },

    goods =
    {
        '金创药（中）',
        '魔法药（中）',
        '金创药（大）',
        '魔法药（大）',
        '金创药（特）',
        '魔法药（特）',
        '太阳水',
        '强效太阳水',
    },

    trade  = {'恢复药水'},

    buyText =
    {
        '出门在外时，多带上些药品心里才踏实。',
    },

    tradeText =
    {
        '你要卖什么药品？',
    },
}
