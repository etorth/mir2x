-- converted from Envir/Market_Def/02Weapon_Encore-9.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎光临！一般的武器我这儿都有。',
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
        '在险恶的江湖，武器就是第二生命，你需要什么样的武器?',
    },

    tradeText =
    {
        '您要出售什么武器? 我会给好价钱的。',
    },

    repairText =
    {
        '虽然我的手艺不是很出众，但是一般的武器我还是可以修理的，只是武器的持久值会有所下降。',
    },
}
