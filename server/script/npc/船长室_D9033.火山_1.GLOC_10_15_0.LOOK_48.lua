-- converted from Envir/Market_Def/02Weapon_SinGiSun-D9012.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '喂，老兄你的武器太旧了。我这里有很多工具，可以修怎么样？稍微贵一点没问题吧？',
    },

    -- legacy offers no trade here
    trade = false,

    repairText =
    {
        '稍微等一下我马上给你修。',
    },

    repairDone = '这程度还算满意。',

    -- this one does not offer 特殊修理
    special = false,
}
