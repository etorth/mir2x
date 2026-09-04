-- converted from Envir/Market_Def/04Potion_Oasis-4.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '最近外地人常来。你要的是什么来着？',
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
    },

    buyText =
    {
        '请选择你所需要的。',
    },

    sellText =
    {
        '你想卖什么药？',
    },

    today = '今天没事情可拜托你了。',

    topics =
    {
        -- legacy @Wjwn
        {
            id    = 'npc_wjwn',
            label = '购买',
            text  = {'……'},   -- legacy section carries no prose
        },
    },
}
