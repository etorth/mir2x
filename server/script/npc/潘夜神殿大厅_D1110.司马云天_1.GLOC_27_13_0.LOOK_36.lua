-- converted from Envir/Market_Def/02Weapon_HalfTemple-D1110.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '欢迎光临。。 我们全家人都在这里做事。',
        '用心做事肯定能赚到很多钱的。。 有需要做特殊修理的东西就交给我吧。',
    },

    -- legacy offers no trade or repair here
    trade = false,
    repair = false,
}
