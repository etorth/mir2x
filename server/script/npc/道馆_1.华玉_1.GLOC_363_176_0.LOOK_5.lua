-- converted from Envir/Market_Def/04Potion_Wooma-1.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    goods =
    {
        '金创药（小）',
        '魔法药（小）',
        '蜡烛',
        '地牢逃脱卷',
    },

    trade  = {'恢复药水', '火把', '传送卷轴'},

    buyText =
    {
        '需要什么东西？',
        '这里有传送注文，你买几个吧。',
    },

    tradeText =
    {
        '把你要卖的东西给我，我给你个好价钱。',
        '在我们店里买些药和蜡烛什么的吧。',
    },
}
