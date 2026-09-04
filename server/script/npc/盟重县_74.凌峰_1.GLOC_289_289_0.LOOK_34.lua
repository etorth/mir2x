-- converted from Envir/Market_Def/04Potion_Mongchon-74.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临，为了保证旅途顺利，你应该多准备些药品',
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
        '你要卖什么药品？',
    },

    today = '今天没事情可拜托你了。',
}
