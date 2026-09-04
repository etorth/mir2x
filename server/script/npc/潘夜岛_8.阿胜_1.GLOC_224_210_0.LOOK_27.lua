-- converted from Envir/Market_Def/02Weapon_HalfNight-8.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临，你需要什么？',
    },

    redName = '有什么事？我跟你无话可说。',

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
        '要想在这种偏僻的地方生存下去，必须借助精良的武器，来看一下吧。',
    },

    sellText =
    {
        '请把你不用的武器卖给我。',
    },

    repairText =
    {
        '我可以给你修理武器，不过持久性可能会降低，这我也没办法。',
    },

    repairDone = '这已经算修得不错的了，拿走吧。',
}
