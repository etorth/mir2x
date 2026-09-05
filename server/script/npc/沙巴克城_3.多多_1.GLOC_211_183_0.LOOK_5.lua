-- converted from Envir/Market_Def/08Accessory_Sabuk-3.txt

local jeweler = require('npc.include.merchant.jeweler')
jeweler.setJeweler
{
    -- Castle-war state is not exposed by the server; use the legacy peacetime branch.
    greet = function(uid)
        return
        {
            '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t> 行会的领地。',
            '欢迎光临，本店专门经营饰品。你想买什么样的饰品？',
        }
    end,

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '铁手镯',
        '小手镯',
        '银手镯',
        '钢手镯',
        '大手镯',
        '小手镯',
        '道士手镯',
        '黑檀手镯',
        '金项链',
        '传统项链',
        '灯笼项链',
        '白色虎齿项链',
        '黑色水晶项链',
        '魔鬼项链',
        '蓝翡翠项链',
        '黄色水晶项链',
        '真善项链',
        '黑檀项链',
        '琥珀项链',
        '指环',
        '牛角戒指',
        '蓝色水晶戒指',
        '古铜戒指',
        '黑色水晶戒指',
        '骷髅戒指',
        '水晶魔戒',
        '珍珠戒指',
        '六绝星环',
        '玄铁指环',
        '蛇眼戒指',
    },

    buyText =
    {
        '来~~挑选适合自己的饰品啊。',
    },

    sellText =
    {
        '你想出售饰品？',
        '顺便说一下，本店还经营手套。',
    },

    repairText =
    {
        '可以修理饰品，手套和皮革盔甲。',
    },

    repairDone = '修好了。',
}
