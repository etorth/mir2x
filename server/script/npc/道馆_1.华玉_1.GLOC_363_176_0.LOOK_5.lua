-- converted from Envir/Market_Def/04Potion_Wooma-1.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    redName = '我不愿意和你这样的人进行交易。',

    goods =
    {
        '金创药（小）',
        '魔法药（小）',
        '蜡烛',
        '地牢逃脱卷',
    },

    buyText =
    {
        '需要什么东西？',
        '这里有传送注文，你买几个吧。',
    },

    sellText =
    {
        '把你要卖的东西给我，我给你个好价钱。',
        '在我们店里买些药和蜡烛什么的吧。',
    },

    today = '今天没事情可拜托你了。',
}
