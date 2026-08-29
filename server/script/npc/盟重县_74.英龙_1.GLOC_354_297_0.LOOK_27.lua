-- converted from Envir/Market_Def/02Weapon_Mongchon-74.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎光临，你要参观一下我制造的武器吗？',
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
        '在这种危险的地方，武器就是我的第二生命，你想用这里面的哪种武器？',
    },

    tradeText =
    {
        '你想卖哪种武器？收购价钱不会太高，你还是好好想想再决定吧。',
    },

    repairText =
    {
        '虽然我的手艺不太好，不过一般的武器都能修。可是武器的持久性可能会有所损伤。',
    },
}
