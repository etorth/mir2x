-- converted from Envir/Market_Def/03Armor_Kugkyung-01.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '欢迎光临，有什么事吗？',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '青铜头盔',
        '魔法头盔',
        '布衣（男）',
        '布衣（女）',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
    },

    buyText =
    {
        '你要买什么？',
    },

    sellText =
    {
        '把要出售的防御工具拿出来，我来估估价。',
        '这里头盔和帽子都收购，就在这儿卖吧。',
    },

    preRepairText =
    {
        '确实要修理吗？',
    },

    repairText =
    {
        '防御工具，头盔和帽子都可以修理。',
    },

    repairDone = '修得不错。',

    today = '今天没事情可拜托你了。',
}
