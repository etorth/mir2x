-- converted from Envir/Market_Def/05Book_Encore-9.txt

local bookseller = require('npc.include.merchant.bookseller')
bookseller.setBookseller
{
    greet =
    {
        '欢迎光临！您需要武功秘籍吗?',
    },

    redName = '我不想和你这种人打交道。',

    goods =
    {
        '基本剑术',
        '火球术',
        '治愈术',
    },

    buyText =
    {
        '请挑选您要购买的书。',
    },

    sellText =
    {
        '请把要出售的书放在上面。',
    },
}
