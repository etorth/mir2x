-- converted from Envir/Market_Def/08Accessory_Mongchon-74.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '饰品',

    greet =
    {
    },

    goods =
    {
        '骷髅戒指',
        '小手镯',
        '蓝翡翠项链',
        '珍珠戒指',
        '道士手镯',
        '蛇眼戒指',
        '黑檀手镯',
    },

    trade  = {'戒指', '手镯', '项链'},
    repair = {'戒指', '手镯', '项链'},

    buyText =
    {
        '你想买饰品? 想买什么？请先看好价钱和持久性再决定。',
    },

    tradeText =
    {
        '请先把东西拿出来给我看看。',
    },

    repairText =
    {
        '别以为我年纪小就小看我，我从小就做这一行，绝对没问题。',
    },
}
