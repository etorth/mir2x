-- converted from Envir/Market_Def/04Potion_SabukWar-3.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '开战之前快点卖药赚它一笔。呵呵呵。',
        '你赶紧点儿。我也得准备避难啊。',
        '如果现在不买点药放在仓库的话，你还得老远的跑到毒蛇山谷去呢。呵呵~',
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
    },

    -- legacy offers no trade here
    trade = false,

    buyText =
    {
        '快点啊 快点我也得去避难。',
    },
}
