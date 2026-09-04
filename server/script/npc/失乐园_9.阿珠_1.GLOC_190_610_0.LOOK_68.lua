-- converted from Envir/Market_Def/08Accessory_Encore-9.txt

local jeweler = require('npc.include.merchant.jeweler')
jeweler.setJeweler
{
    greet =
    {
        '我这里有什么样的装饰品都有，您可以随便挑。',
    },

    redName = '我无法和你这种人打交道。',

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
        '您要购买装饰品?想要什么样的?',
    },

    sellText =
    {
        '让我看看您要出售什么物品。',
    },

    repairText =
    {
        '您可别小看我，我可是很在行的。',
    },

    repairDone = '已经修好了。',
}
