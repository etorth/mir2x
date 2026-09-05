-- converted from Envir/Market_Def/04Potion_SnakeVally-2.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t><t color="red">行会的领地。 </t>',
        '欢迎光临，有什么事吗？',
    },

    redName = '我不愿意和你这样的人进行交易。',
    redNameExit = '关闭',

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
        '你想卖什么东西？',
    },

    sellText =
    {
        '请把要卖的东西交给我。',
    },

    today = '今天没事情可拜托你了。',
}
