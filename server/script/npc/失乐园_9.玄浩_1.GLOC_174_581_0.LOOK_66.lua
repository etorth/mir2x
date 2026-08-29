-- converted from Envir/Market_Def/05Book_Encore-9.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '技能书',

    greet =
    {
        '欢迎光临！您需要武功秘籍吗?',
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
        '请挑选您要购买的书。',
    },

    tradeText =
    {
        '请把要出售的书放在上面。',
    },
}
