-- converted from Envir/Market_Def/04Potion_HalfNight-8.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '你来潘夜岛是来对了。要我帮忙吗？',
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
        '别忘了在危急的时候，只有药品才能救活你的性命。',
    },

    sellText =
    {
        '请把你不用的药卖给我吧。',
    },
}
