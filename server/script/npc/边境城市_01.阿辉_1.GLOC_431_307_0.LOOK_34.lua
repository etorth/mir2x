-- converted from Envir/Market_Def/10Material_Kugkyung-01.txt

local buyer = require('npc.include.merchant.buyer')
buyer.setBuyer
{
    greet =
    {
        '欢迎光临，请卖给我蚂蚁卵或者骷髅骨之类的材料。',
    },

    redName = '我不愿意和你这样的人进行交易。',

    sellText =
    {
        '你要出售什么？',
    },

    today = '今天没事情可拜托你了。',
}
