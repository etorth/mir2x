-- converted from Envir/Market_Def/02Weapon_Kugkyung-01.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎光临，感谢光临。',
    },

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

    trade  = {'武器'},
    repair = {'武器'},
    special = true,

    buyText =
    {
        '请选择要购买的武器。',
    },

    tradeText =
    {
        '请把要出售的武器抬上来。',
    },

    repairText =
    {
        '你要修理武器？请把你要修理的东西抬上来。',
    },
}
