-- converted from Envir/Market_Def/02Weapon_SnakeVally-2.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '武器',

    greet =
    {
        '这里是 沙巴克城  行会的领地。',
        '欢迎，感谢光临毒蛇山谷的铁匠铺。',
    },

    goods =
    {
        '鹤嘴锄',
        '风之鹤嘴锄',
    },

    trade  = {'武器'},
    repair = {'武器'},
    special = true,

    buyText =
    {
        '请选择要购买的武器。',
    },

    tradeText =
    {
        '请把要出售的武器抬上来。',
    },

    repairText =
    {
        '确实要修理武器吗？',
    },
}
