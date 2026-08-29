-- converted from Envir/Market_Def/03Armor_Samak-5.txt

local shop = require('npc.include.shop')
shop.setMerchant
{
    label = '防御工具',

    greet =
    {
        '欢迎光临沙漠土城， 有什么需要我帮忙的吗？',
    },

    goods =
    {
        '魔法头盔',
        '道士头盔',
        '轻型盔甲（男）',
        '轻型盔甲（女）',
        '重盔甲（男）',
        '重盔甲（女）',
        '灵魂战衣（男）',
        '灵魂战衣（女）',
        '魔法长袍（男）',
        '魔法长袍（女）',
    },

    trade  = {'头盔', '衣服'},
    repair = {'头盔', '衣服'},

    buyText =
    {
        '慢慢看，这里型号齐全。',
    },

    tradeText =
    {
        '这里的天气很热，但是冒然脱掉衣服有损健康。',
    },

    repairText =
    {
        '我对我的维修手艺很有信心，你试试看吧。',
    },
}
