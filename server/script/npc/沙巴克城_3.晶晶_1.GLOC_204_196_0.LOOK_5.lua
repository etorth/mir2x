-- converted from Envir/Market_Def/04Potion_Sabuk-3.txt

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
    },

    trade  = {'恢复药水'},

    buyText =
    {
        '选好需要的东西了吗？快点选，年轻人怎么还那么慢吞吞的。',
    },

    tradeText =
    {
        '你想卖东西？真是的，本来生意就不好。 快点卖了走吧。',
    },
}
