-- converted from Envir/Market_Def/04Potion_Eunhang-02.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临，你需要什么？',
    },

    redName = '我不愿意和你这样的人进行交易。',

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

    buyText =
    {
        '你需要什么？',
    },

    sellText =
    {
        '把你要出售的东西给我看看，我来估估价。',
    },

    today = '今天没事情可拜托你了。',
}
