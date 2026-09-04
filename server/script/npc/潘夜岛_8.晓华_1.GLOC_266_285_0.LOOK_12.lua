-- converted from Envir/Market_Def/08Accessory_HalfNight-8.txt

local jeweler = require('npc.include.merchant.jeweler')
jeweler.setJeweler
{
    greet =
    {
        '欢迎光临，你需要什么？',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '骷髅戒指',
        '小手镯',
        '蓝翡翠项链',
        '珍珠戒指',
        '道士手镯',
        '蛇眼戒指',
        '黑檀手镯',
    },

    buyText =
    {
        '你想买饰品？买什么？仔细挑选一下吧。',
    },

    sellText =
    {
        '请把不用的饰品卖给我。',
    },

    repairText =
    {
        '那我给你修吧。你要修什么？',
    },

    repairDone = '修好了。',

    today = '今天没事情可拜托你了。',
}
