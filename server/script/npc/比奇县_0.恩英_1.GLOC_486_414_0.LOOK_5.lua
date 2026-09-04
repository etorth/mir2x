-- converted from Envir/Market_Def/04Potion_Bichon2-0.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临，这里出售一些简单的药品。',
    },

    redName = '我不愿意和你这样的坏人进行交易。',

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
    },

    buyText =
    {
        '你需要什么东西？',
    },

    sellText =
    {
        '请把要出售的物品交给我。',
    },
}
