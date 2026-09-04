-- converted from Envir/Market_Def/04Potion_Sabuk-3.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '在沙巴克城正展开攻城阵都不能做生意了。。。所以到这儿避难来了。',
        '什么，这里是卖药的地方? 你已经知道了? 那你需要什么，快点买走吧。',
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

    buyText =
    {
        '选好需要的东西了吗？快点选，年轻人怎么还那么慢吞吞的。',
    },

    sellText =
    {
        '你想卖东西？真是的，本来生意就不好。 快点卖了走吧。',
    },
}
