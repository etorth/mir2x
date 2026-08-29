-- converted from Envir/Market_Def/03Armor_Mongchon-74.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '既然来了，买点儿新的防御工具再走吧。',
    },

    goods =
    {
        '魔法头盔',
        '重盔甲（男）',
        '重盔甲（女）',
        '灵魂战衣（男）',
        '灵魂战衣（女）',
        '魔法长袍（男）',
        '魔法长袍（女）',
    },

    trade  = {'头盔', '衣服'},
    repair = {'头盔', '衣服'},

    buyText =
    {
        '慢慢看，别着急。',
    },

    tradeText =
    {
        '你想卖什么防御工具，我可以给你个好价钱。',
    },

    repairText =
    {
        '等我把它弄好了，就跟新衣服一样。',
    },
}
