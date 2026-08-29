-- converted from Envir/Market_Def/02Weapon_HalfNight-8.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎光临，你需要什么？',
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
    },

    trade  = {'武器'},
    repair = {'武器'},
    special = true,

    buyText =
    {
        '要想在这种偏僻的地方生存下去，必须借助精良的武器，来看一下吧。',
    },

    tradeText =
    {
        '请把你不用的武器卖给我。',
    },

    repairText =
    {
        '我可以给你修理武器，不过持久性可能会降低，这我也没办法。',
    },
}
