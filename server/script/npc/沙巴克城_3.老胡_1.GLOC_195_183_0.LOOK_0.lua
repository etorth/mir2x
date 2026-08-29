-- converted from Envir/Market_Def/02Weapon_Sabuk-3.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '在沙巴克城正展开攻城阵都不能做生意了。。。所以到这儿避难来了。',
        '欢迎光临，你需要哪类武器？这儿有很多既便宜又结实的剑，你随便选。',
    },

    goods =
    {
        '黑铁矿',
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
        '种类很多，你随便挑。',
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
