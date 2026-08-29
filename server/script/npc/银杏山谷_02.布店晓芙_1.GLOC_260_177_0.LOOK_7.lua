-- converted from Envir/Market_Def/03Armor_Eunhang-02.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '欢迎光临，你需要什么？',
    },

    goods =
    {
        '青铜头盔',
        '魔法头盔',
        '布衣（男）',
        '布衣（女）',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
    },

    trade  = {'头盔', '衣服'},
    repair = {'头盔', '衣服'},

    buyText =
    {
        '你要买什么？',
    },

    tradeText =
    {
        '你要买什么？',
    },

    repairText =
    {
        '防御工具，头盔和帽子都可以修理。',
    },
}
