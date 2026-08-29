-- converted from Envir/Market_Def/08Accessory_Encore-9.txt

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
        '您要购买装饰品?想要什么样的?',
    },

    tradeText =
    {
        '让我看看您要出售什么物品。',
    },

    repairText =
    {
        '您可别小看我，我可是很在行的。',
    },
}
