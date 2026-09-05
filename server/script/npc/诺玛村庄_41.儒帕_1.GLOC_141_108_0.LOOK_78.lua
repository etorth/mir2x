-- converted from Envir/Market_Def/03Armor_Numa-41.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    greet =
    {
        '虽然我进货的时候比较仓促,但所进的都是好货色. 尽管挑吧.',
    },

    redName = '我不想跟你这种人进行交易...',
    redNameExit = '关闭',

    label = '',
    buyLabel = '买防具',
    sellLabel = '卖防具',
    repairLabel = '修理防具',
    backLabel = '继续',
    exitLabel = '关闭',

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
        '人类勇士总认为这个价格太贵. 但如果他们知道弄这些东西有多困难,就应该能接受这个价格. 少一分钱我也不会卖.',
    },

    sellText =
    {
        '我们诺玛族已经习惯了沙漠的气候,根本就不用穿多余的衣服.但人类不同... 反正,先让我看看你的东西.',
    },

    repairText =
    {
        '想修理旧衣服? 怎么,你怀疑我的技术吗? 我虽然刚刚开始修理人类的衣服,但以我多年修理诺玛族盔甲的经验来看,修理人类的衣服也没问题.',
    },

    repairDone = '修理完成.',
    repairDoneBack = '继续',
}
