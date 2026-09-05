-- converted from Envir/Market_Def/03Shoes_Bichon-0.txt

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
        '草鞋',
        '皮靴',
    },

    label = '鞋',
    trade = {'鞋'},
    repair = {'鞋'},
    repairSuffix = '',

    buyText =
    {
        '你要买什么样的鞋？',
    },

    sellText =
    {
        '请把不要的东西卖给我吧。',
    },

    repairText =
    {
        '可以修鞋。',
    },

    repairDone = '修得不错。',

    today = '今天没事情可拜托你了。',
}
