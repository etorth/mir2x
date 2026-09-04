-- converted from Envir/Market_Def/02Weapon_Encore-9.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临！一般的武器我这儿都有。',
    },

    redName = '我不想和你这种人打交道。。',

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
        '在险恶的江湖，武器就是第二生命，你需要什么样的武器?',
    },

    sellText =
    {
        '您要出售什么武器? 我会给好价钱的。',
    },

    repairText =
    {
        '虽然我的手艺不是很出众，但是一般的武器我还是可以修理的，只是武器的持久值会有所下降。',
    },

    repairDone = '好了，修得差不多了。',
}
