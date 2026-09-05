-- converted from Envir/Market_Def/03Armor_Sabuk-3.txt

local outfitter = require('npc.include.merchant.outfitter')
outfitter.setOutfitter
{
    -- Castle-war state is not exposed by the server; use the legacy peacetime branch.
    greet = function(uid)
        return
        {
            '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t> 行会的领地。',
            '欢迎光临，我们店里有各式各样的衣服。你随便挑选。',
        }
    end,

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '布衣（男）',
        '布衣（女）',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
        '重盔甲（男）',
        '重盔甲（女）',
        '灵魂战衣（男）',
        '灵魂战衣（女）',
        '魔法长袍（男）',
        '魔法长袍（女）',
    },

    buyText =
    {
        '随便挑，随便选，这件衣服怎么样？',
    },

    sellText =
    {
        '请把要卖的衣服拿上来，我来估估价。',
        '头盔和帽子在旁边那家店里卖，',
        '你去那儿看看吧。',
    },

    repairText =
    {
        '衣服穿得很破了，还是修理一下吧。',
    },

    repairDone = '修得不错。',
}
