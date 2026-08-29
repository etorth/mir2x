-- converted from Envir/Market_Def/02Weapon_Eunhang-02.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎，感谢光临。',
    },

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

    trade  = {'武器'},
    repair = {'武器'},
    special = true,

    buyText =
    {
        '请选择要购买的武器。',
    },

    tradeText =
    {
        '请把要卖的武器抬上来。',
    },

    repairText =
    {
        '你要修理武器？把要修的武器抬上来。',
    },
}
