-- converted from Envir/Market_Def/04Potion_Kugkyung-01.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临。 请随便挑选。',
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
        '你想卖什么？',
    },

    sellText =
    {
        '请把要出售的东西拿上来。',
    },

    today = '今天没事情可拜托你了。',
}
