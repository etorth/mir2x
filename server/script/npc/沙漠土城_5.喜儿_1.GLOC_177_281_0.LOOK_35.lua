-- converted from Envir/Market_Def/03Armor_Samak-5.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '欢迎光临沙漠土城， 有什么需要我帮忙的吗？',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '魔法头盔',
        '道士头盔',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
        '重盔甲（男）',
        '重盔甲（女）',
        '灵魂战衣（男）',
        '灵魂战衣（女）',
        '魔法长袍（男）',
        '魔法长袍（女）',
    },

    buyText =
    {
        '慢慢看，这里型号齐全。',
    },

    sellText =
    {
        '这里的天气很热，但是冒然脱掉衣服有损健康。',
    },

    repairText =
    {
        '我对我的维修手艺很有信心，你试试看吧。',
    },

    repairDone = '来，穿一下看看',
}
