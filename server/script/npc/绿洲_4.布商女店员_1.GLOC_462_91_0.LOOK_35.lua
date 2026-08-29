-- converted from Envir/Market_Def/03Armor_Oasis-4.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '祝你一路平安。有什么事吗？',
    },

    goods =
    {
        '重盔甲（男）',
        '重盔甲（女）',
        '灵魂战衣（男）',
        '灵魂战衣（女）',
        '魔法长袍（男）',
        '魔法长袍（女）',
    },

    trade  = {'衣服'},
    repair = {'衣服'},

    buyText =
    {
        '在这穷乡僻壤里，不知道有没有你中意的衣服，进来看看吧。',
    },

    tradeText =
    {
        '那么，你带什么衣服来了？最好是我们村子里找不到的东西。',
    },

    repairText =
    {
        '衣服和头盔看起来很旧啊，我给你弄干净弄好，别客气，给我吧。',
    },
}
