-- converted from Envir/Market_Def/02Weapon_Bichon2-0.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '很高兴见到你，有什么事吗？',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '青铜斧',
        '八荒',
        '凌风',
        '斩马刀',
        '修罗',
        '海魂',
        '半月',
        '鹤嘴锄',
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
        '要修理武器吗？',
    },

    repairDone = '修好了。',

    today = '今天没事情可拜托你了。',
    qweapon = true,
    qweaponSuffix = '有关武器的事',
}
