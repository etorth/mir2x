-- converted from Envir/Market_Def/04Potion_Oasis-4.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
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
    },

    trade  = {'恢复药水'},

    buyText =
    {
        '请选择你所需要的。',
    },

    tradeText =
    {
        '你想卖什么药？',
    },
}
