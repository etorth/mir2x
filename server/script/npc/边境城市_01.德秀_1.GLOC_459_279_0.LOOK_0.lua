-- converted from Envir/Market_Def/02Weapon_Kugkyung-01.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临，感谢光临。',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '木剑',
        '匕首',
        '青铜剑',
        '铁剑',
        '乌木剑',
        '青铜斧',
        '八荒',
        '凌风',
        '斩马刀',
        '修罗',
    },

    buyText =
    {
        '请选择要购买的武器。',
    },

    sellText =
    {
        '请把要出售的武器抬上来。',
    },

    preRepairText =
    {
        '请把要修理的武器放上去。',
    },

    repairText =
    {
        '你要修理武器？请把你要修理的东西抬上来。',
    },

    repairDone = '修得不错，我下次再来。',

    today = '今天没事情可拜托你了。',
}
