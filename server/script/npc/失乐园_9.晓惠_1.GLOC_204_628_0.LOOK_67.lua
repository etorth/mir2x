-- converted from Envir/Market_Def/03Armor_Encore-9.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '欢迎光临！您需要什么吗?',
    },

    goods =
    {
        '道士头盔',
        '战神盔甲（男）',
        '战神盔甲（女）',
        '幽灵战衣（男）',
        '幽灵战衣（女）',
        '恶魔长袍（男）',
        '恶魔长袍（女）',
    },

    trade  = {'头盔', '衣服'},
    repair = {'头盔', '衣服'},

    buyText =
    {
        '慢慢看，什么尺寸都有。',
    },

    tradeText =
    {
        '这里天气炎热，不穿衣服会有损健康。',
    },

    repairText =
    {
        '修补衣服我最在行了，包在我身上。',
    },
}
