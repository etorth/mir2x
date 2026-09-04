-- converted from Envir/Market_Def/10Material_SnakeVally-2.txt

local buyer = require('npc.include.merchant.buyer')
buyer.setBuyer
{
    greet =
    {
        '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t><t color="red">行会的领地。 </t>',
        '欢迎光临，请卖给我蚂蚁卵或者骷髅骨之类的材料。',
    },

    redName = '我不愿意和你这样的人进行交易。',

    sellText =
    {
        '你要出售什么？',
    },
}
