-- converted from Envir/Market_Def/03Armor_Mongchon-74.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '既然来了，买点儿新的防御工具再走吧。',
    },

    redName = '我不想和你这种坏人做生意。',
    redNameExit = '关闭',

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

    buyText =
    {
        '慢慢看，别着急。',
    },

    sellText =
    {
        '你想卖什么防御工具，我可以给你个好价钱。',
    },

    repairText =
    {
        '等我把它弄好了，就跟新衣服一样。',
    },

    repairDone = '好了，弄完了。你要试试吗？',

    today = '今天没事情可拜托你了。',
}
