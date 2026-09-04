-- converted from Envir/Market_Def/04Potion_SinGiSun-D9011.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '见到你真好. 这里四处都是怪物, 我很担心… 我想把药水卖完之后 赶快离开这儿. 虽然比村庄贵一些, 可是你来之前 我都给人家卖3倍的价格, 你到其他地方买不到这个价格.',
    },

    goods =
    {
        '金创药（大）',
        '魔法药（大）',
        '金创药（特）',
        '魔法药（特）',
        '强效太阳水',
        '万年雪霜',
    },

    -- legacy offers no trade here
    trade = false,

    buyText =
    {
        '你快点挑啊？挑好了我才能去继续练级。',
    },
}
