-- converted from Envir/Market_Def/03Shoes_Samak-5.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '鞋',

    greet =
    {
    },

    goods =
    {
        '草鞋',
        '皮靴',
    },

    trade  = {'鞋'},
    repair = {'鞋'},

    buyText =
    {
        '请挑选你需要的鞋。',
    },

    tradeText =
    {
        '据说在沙漠这样危险的地方，鞋是必需品。',
    },

    repairText =
    {
        '可以修理鞋。',
    },
}
