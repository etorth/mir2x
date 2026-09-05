-- converted from Envir/Market_Def/02Weapon_Sabuk-3.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    -- Castle-war state is not exposed by the server; use the legacy peacetime branch.
    greet = function(uid)
        return
        {
            '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t> 行会的领地。',
            '欢迎光临，你需要哪类武器？这儿有很多既便宜又结实的剑，你随便选。',
        }
    end,

    redName = '我不想和你这种坏人做生意。',

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

    buyText =
    {
        '种类很多，你随便挑。',
    },

    sellText =
    {
        '请把要出售的武器抬上来。',
    },

    repairText =
    {
        '要修理武器吗？',
    },

    repairDone = '你要修理武器吗？修哪种？',
}
