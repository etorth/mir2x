-- converted from Envir/Market_Def/03Armor_Bichon-0.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    goods =
    {
        '青铜头盔',
        '魔法头盔',
        '布衣（男）',
        '布衣（女）',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
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
        '你要买什么？',
    },

    tradeText =
    {
        '请把要出售的衣服拿出来，我来估估价。',
        '这里头盔和帽子都收购， 就在这儿卖吧。',
    },

    repairText =
    {
        '衣服，头盔和帽子都可以修理。',
    },
}
