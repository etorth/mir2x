-- converted from Envir/Market_Def/02Weapon_SnakeVally-2.txt

local smith = require('npc.include.merchant.smith')
smith.setSmith
{
    greet =
    {
        '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t><t color="red">行会的领地。 </t>',
        '欢迎，感谢光临毒蛇山谷的铁匠铺。',
    },

    redName = '我不想和你这种坏人做生意。',

    goods =
    {
        '鹤嘴锄',
        '风之鹤嘴锄',
    },

    buyText =
    {
        '请选择要购买的武器。',
    },

    sellText =
    {
        '请把要出售的武器抬上来。',
    },

    preRepairText =
    {
        '请把要修理的武器放上去。',
    },

    repairText =
    {
        '确实要修理武器吗？',
    },

    repairDone = '你要修理武器？请把你要修理的东西抬上来。',

    today = '今天没事情可拜托你了。',
}
