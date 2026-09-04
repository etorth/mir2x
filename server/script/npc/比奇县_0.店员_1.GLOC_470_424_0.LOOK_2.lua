-- converted from Envir/Market_Def/05Book_Bichon-0.txt

local bookseller = require('npc.include.merchant.bookseller')
bookseller.setBookseller
{
    greet =
    {
        '欢迎光临，你来买练武功的书？',
    },

    redName = '我不愿意和你这样的人进行交易。',

    goods =
    {
        '基本剑术',
        '火球术',
        '治愈术',
    },

    buyText =
    {
        '请挑选你想要的书。',
    },

    sellText =
    {
        '请把要出售的物品拿上来。',
    },

    today = '今天没事情可拜托你了。',

    -- legacy @NPC_HelpBooks, the full seven-book menu
    books =
    {
        '基本剑术',
        '半月弯刀',
        '火球术',
        '霹雳掌',
        '治愈术',
        '精神力战法',
        '施毒术',
    },
}
