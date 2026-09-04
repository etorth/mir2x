-- converted from Envir/Market_Def/04Potion_Samak-5.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临，我们药店愿竭诚为你服务。',
    },

    redName = '我不愿意和你这样的人进行交易。',

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

    buyText =
    {
        '出门在外时，多带上些药品心里才踏实。',
    },

    sellText =
    {
        '你要出售什么药品？',
    },
}
