-- converted from Envir/Market_Def/02Weapon_Bichon2-0.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '很高兴见到你，有什么事吗？',
    },

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
        '要修理武器吗？',
    },
}
