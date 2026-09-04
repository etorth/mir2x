-- converted from Envir/Market_Def/03Armor_Bichon-0.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    redName = '像你这样杀气腾腾的人站在商店门口，其他人不敢进来，请你走开',

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

    buyText =
    {
        '你要买什么？',
    },

    sellText =
    {
        '请把要出售的衣服拿出来，我来估估价。',
        '这里头盔和帽子都收购， 就在这儿卖吧。',
    },

    repairText =
    {
        '确实要修理吗？',
        '衣服，头盔和帽子都可以修理。',
    },

    repairDone = '修得不错。',

    today = '今天没事情可拜托你了。',
}
