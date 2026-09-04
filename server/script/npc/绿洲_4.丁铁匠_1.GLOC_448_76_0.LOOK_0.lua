-- converted from Envir/Market_Def/02Weapon_Oasis-4.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临，异乡人。 你需要什么？',
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
        '明明有能力使用攻击力强的武器，却非要使用攻击力弱的武器，证明你没有竭尽全力，我们村子不欢迎那种人。那么，你想买什么？',
    },

    sellText =
    {
        '我把你卖的武器修理好，弄干净，然后给其他人用。你带来了什么东西，拿出来给我看看。',
    },

    repairText =
    {
        '在我们这里，武器常修不常换是一种美德。我们村子里的武器都是我修的，你也让我修吧。',
    },

    repairDone = '这已经算修得不错的了，拿走吧。',

    today = '今天没事情可拜托你了。',
}
