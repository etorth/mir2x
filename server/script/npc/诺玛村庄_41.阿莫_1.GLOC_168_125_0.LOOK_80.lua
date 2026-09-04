-- converted from Envir/Market_Def/04Potion_Numa-41.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '我这里卖的药品跟人类药店里卖的药没什么不同,放心买吧.',
    },

    redName = '治愈你这种邪恶的人,上天都会惩罚我的!',

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
        '你到底想买什么药?',
    },

    sellText =
    {
        '你想出售药水? 也好.不过先让我看看你要卖的东西,因为我出的价格是按照药水品质而定的.',
    },
}
