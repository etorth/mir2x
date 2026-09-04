-- converted from Envir/Market_Def/05Book_Kugkyung-01.txt

local bookseller = require('npc.include.merchant.bookseller')
bookseller.setBookseller
{
    greet =
    {
        '欢迎光临',
    },

    redName = '我不愿意和你这样的人进行交易。',

    goods =
    {
        '基本剑术',
    },

    buyText =
    {
        '请挑选你想要的书。',
    },

    sellText =
    {
        '请把要出售的图书拿上来。',
    },

    today = '今天没事情可拜托你了。',

    -- legacy @NPC_QuestionPrize, a warrior-book shop under its own label
    books =
    {
        '基本剑术',
        '半月弯刀',
    },

    booksLabel  = '询问',
    booksSuffix = '有关物品的事',
}
