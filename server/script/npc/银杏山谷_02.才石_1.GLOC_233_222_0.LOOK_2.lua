-- converted from Envir/Market_Def/05Book_Eunhang-02.txt

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
        '火球术',
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

    -- legacy @NPC_HelpBooks, a mage-book shop
    books =
    {
        '火球术',
        '霹雳掌',
    },
}
