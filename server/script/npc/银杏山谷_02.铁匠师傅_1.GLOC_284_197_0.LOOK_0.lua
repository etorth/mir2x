-- converted from Envir/Market_Def/02Weapon_Eunhang-02.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎，感谢光临。',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '木剑',
        '匕首',
        '青铜剑',
        '铁剑',
        '乌木剑',
        '海魂',
        '鹤嘴锄',
        '风之鹤嘴锄',
    },

    buyText =
    {
        '请选择要购买的武器。',
    },

    sellText =
    {
        '请把要卖的武器抬上来。',
    },

    repairText =
    {
        '请把要修理的武器放上去。',
        '你要修理武器？把要修的武器抬上来。',
    },

    repairDone = '修得不错，下次我还来找你修。',

    today = '今天没事情可拜托你了。',
    qweapon = true,
}
