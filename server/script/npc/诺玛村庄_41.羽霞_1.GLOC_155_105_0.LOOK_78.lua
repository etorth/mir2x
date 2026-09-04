-- converted from Envir/Market_Def/08Accessory_Numa-41.txt

local jeweler = require('npc.include.merchant.jeweler')
jeweler.setJeweler
{
    greet =
    {
        '大家来瞧一瞧看一看啊, 这里回收各种项链和戒指,还可以对饰品进行修理. 快来看啊,都是上好的货色.',
    },

    redName = '我可不想跟你这样的坏人进行交易...',

    goods =
    {
        '小手镯',
        '蓝翡翠项链',
        '珍珠戒指',
        '道士手镯',
        '蛇眼戒指',
        '黑檀手镯',
    },

    buyText =
    {
        '这些都是从沙漠土城商人那里采购的货物, 肯定有你需要的.',
    },

    sellText =
    {
        '我这里高价回收旧的饰品. 把你要出售的饰品拿来吧.',
    },

    repairText =
    {
        '你想修理旧饰品. 把要修理的饰品拿来吧.',
        '我的修理技术不会比沙漠土城的工匠差的.',
    },

    repairDone = '修理完成.',

    topics =
    {
        -- legacy @rustaccessory
        {
            id    = 'npc_rustaccessory',
            label = '询问生锈饰品.',
            text  = {'……'},   -- legacy section carries no prose
        },
    },
}
