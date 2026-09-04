-- converted from Envir/Market_Def/08Accessory_Mongchon-74.txt

local jeweler = require('npc.include.merchant.jeweler')
jeweler.setJeweler
{
    greet =
    {
        '欢迎光临，为了保证旅途的安全，你还是事先做好准备吧',
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
        '你想买饰品? 想买什么？请先看好价钱和持久性再决定。',
    },

    sellText =
    {
        '请先把东西拿出来给我看看。',
    },

    repairText =
    {
        '别以为我年纪小就小看我，我从小就做这一行，绝对没问题。',
    },

    repairDone = '修好了。',

    today = '今天没事情可拜托你了。',
}
