-- converted from Envir/Market_Def/05Book_Bichon-0.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '技能书',

    greet =
    {
        '欢迎光临，你来买练武功的书？',
    },

    goods =
    {
        '基本剑术',
        '火球术',
        '治愈术',
    },

    trade  = {'技能书'},

    buyText =
    {
        '请挑选你想要的书。',
    },

    tradeText =
    {
        '请把要出售的物品拿上来。',
    },
}
