-- converted from Envir/Market_Def/03Armor_HalfNight-8.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '一路辛苦了，你需要什么？',
    },

    redName = '我不想和你这种坏人做生意。',

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

    buyText =
    {
        '不用急，慢慢考虑，慢慢挑选。',
    },

    sellText =
    {
        '请把不要的东西卖给我吧。',
    },

    repairText =
    {
        '虽然我的手艺不太好，但我可以给你修理。',
    },

    repairDone = '都弄好了，你穿上试试。',

    today = '今天没事情可拜托你了。',
}
