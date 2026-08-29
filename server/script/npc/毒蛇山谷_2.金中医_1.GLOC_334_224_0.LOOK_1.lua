-- converted from Envir/Market_Def/04Potion_SnakeVally-2.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '药品',

    greet =
    {
        '这里是 沙巴克城  行会的领地。',
        '欢迎光临，有什么事吗？',
    },

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

    trade  = {'恢复药水'},

    tradeText =
    {
        '请把要卖的东西交给我。',
    },
}
