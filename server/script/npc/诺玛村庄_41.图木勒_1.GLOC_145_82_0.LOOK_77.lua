-- converted from Envir/Market_Def/02Weapon_Numa-41.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '我虽然不想跟人类交易,但为了糊口别无选择...',
    },

    redName = '我不想跟你这种人进行交易,快从我眼前消失吧.',

    goods =
    {
        '#INCLUDE',
    },

    buyText =
    {
        '人类铁匠打造的武器决不能和我卖的武器相提并论,我这里都是好货色,你尽管挑吧.',
    },

    sellText =
    {
        '你真的这么急需用钱,竟然到了出售自己武器的地步? 人类只知道钱,钱...',
        '如果真的是急需用钱,就让我来帮你吧. 把要出售的武器抬上来.',
    },

    repairText =
    {
        '勇士一定要好好保养自己的武器. 把你想要修理的武器放上来吧. 我会让它恢复原样.',
    },

    repairDone = '修理完成.',
}
