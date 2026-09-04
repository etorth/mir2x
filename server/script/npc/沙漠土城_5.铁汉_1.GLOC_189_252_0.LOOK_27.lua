-- converted from Envir/Market_Def/02Weapon_Samak-5.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临，一般的武器我们这儿全都有，你先看看',
    },

    redName = '你要打我？？哎呦，太可怕了，呜呜;;',

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

    buyText =
    {
        '在这种危险的地方，武器就是我的第二生命，你想用这里面的哪种武器？',
    },

    sellText =
    {
        '你想卖哪种武器？收购价钱不会太高，你还是好好想想再决定吧。',
    },

    repairText =
    {
        '虽然我的手艺不太好，不过一般的武器都能修。可是武器的持久性可能会有所损伤。',
    },

    repairDone = '这已经算修得不错的了，拿走吧。',
    removeSword = true,
}
