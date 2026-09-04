-- converted from Envir/Market_Def/04Potion_Zuma-D5071.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '我也是历经了千辛万苦才来到这里的。 因此我愿意出3倍的价钱收购你的东西。',
    },

    goods =
    {
        '金创药（大）',
        '魔法药（大）',
        '金创药（特）',
        '魔法药（特）',
        '太阳水',
        '强效太阳水',
        '万年雪霜',
    },

    -- legacy offers no trade here
    trade = false,

    buyText =
    {
        '要想在祖玛寺庙里打猎，你不得带足药品吗？',
    },
}
