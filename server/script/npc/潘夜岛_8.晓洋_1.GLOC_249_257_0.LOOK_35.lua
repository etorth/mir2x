-- converted from Envir/Market_Def/03Armor_HalfNight-8.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '一路辛苦了，你需要什么？',
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
        '不用急，慢慢考虑，慢慢挑选。',
    },

    tradeText =
    {
        '请把不要的东西卖给我吧。',
    },

    repairText =
    {
        '虽然我的手艺不太好，但我可以给你修理。',
    },
}
