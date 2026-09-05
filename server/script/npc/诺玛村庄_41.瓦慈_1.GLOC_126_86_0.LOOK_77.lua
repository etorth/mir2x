-- converted from Envir/Market_Def/01Meet_NuMa-41.txt

local butcher = require('npc.include.merchant.butcher')
butcher.setButcher
{
    greet =
    {
        '我们特别吃喜欢人类饲养的 <t color="red">家畜肉</t><t color="red">. 因为它们的肉比蜥蜴肉嫩多了. </t>',
        '但是人类很小气,不会把肉白白送给我们.所以只能按人类的方式进行现金交易.',
    },

    redName = '我不想跟你这种身上充满血腥味的人类进行交易.',
    redNameExit = '关闭',

    label = '',
    sellLabel = '卖肉',
    backLabel = '继续',
    exitLabel = '关闭',

    sellText =
    {
        '如果是新鲜的肉,我会出高价的. 快拿出来,让我看看.',
        '我快忍不住了.',
    },
}
