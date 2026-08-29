-- converted from Envir/Market_Def/04Potion_Encore-9.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
        '欢迎光临！我们药铺精心准备好各种药品。',
    },

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

    trade  = {'恢复药水'},

    buyText =
    {
        '出门之前，一定准备好充足的药品。',
    },

    tradeText =
    {
        '您要出售什么药品?',
    },
}
