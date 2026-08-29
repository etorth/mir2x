-- converted from Envir/Market_Def/03Armor_SankeVally-2.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '这里是 沙巴克城  行会的领地。',
        '欢迎光临，有什么事吗？',
    },

    goods =
    {
        '青铜头盔',
        '魔法头盔',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
    },

    trade  = {'头盔', '衣服'},
    repair = {'头盔', '衣服'},

    buyText =
    {
        '你要买什么？',
    },

    tradeText =
    {
        '把要出售的防御工具拿出来，我来估估价。',
        '这里头盔和帽子都收购， 就在这儿卖吧。',
    },

    repairText =
    {
        '防御工具，头盔和帽子都可以修理。',
    },
}
