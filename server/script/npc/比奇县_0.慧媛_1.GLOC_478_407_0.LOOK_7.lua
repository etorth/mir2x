-- converted from Envir/Market_Def/03Shoes_Bichon-0.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '鞋',

    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    goods =
    {
        '草鞋',
        '皮靴',
    },

    trade  = {'鞋'},
    repair = {'鞋'},

    buyText =
    {
        '你要买什么样的鞋？',
    },

    tradeText =
    {
        '请把不要的东西卖给我吧。',
    },

    repairText =
    {
        '可以修鞋。',
    },
}
