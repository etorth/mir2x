-- converted from Envir/Market_Def/01Meet_DoGwan-1.txt

local butcher = require('npc.include.merchant.butcher')
butcher.setButcher
{
    greet =
    {
        '这里是 沙巴克城 <t color="red">' .. getSubukGuildName() .. '</t><t color="red">行会的领地。 </t>',
        '你是来卖肉的？',
    },

    redName = '我不想和你这种坏人做生意。',

    sellText =
    {
        '高价收购优质肉。',
        '沾上土的或被火烧过的肉廉价收购。',
    },

    today = '今天没事情可拜托你了。',
}
