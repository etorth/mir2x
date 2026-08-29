-- converted from Envir/Market_Def/02Weapon_Oasis-4.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '欢迎光临，异乡人。 你需要什么？',
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
        '明明有能力使用攻击力强的武器，却非要使用攻击力弱的武器，证明你没有竭尽全力，我们村子不欢迎那种人。那么，你想买什么？',
    },

    tradeText =
    {
        '我把你卖的武器修理好，弄干净，然后给其他人用。你带来了什么东西，拿出来给我看看。',
    },

    repairText =
    {
        '在我们这里，武器常修不常换是一种美德。我们村子里的武器都是我修的，你也让我修吧。',
    },
}
