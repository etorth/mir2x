-- converted from Envir/Market_Def/04Potion_RedZone-81.txt

local apothecary = require('npc.include.merchant.apothecary')
apothecary.setApothecary
{
    greet =
    {
        '我是这里的商人。',
        '这里环境艰苦，相信我这里一定有你需要的东西。',
        '<关闭/exit>',
    },

    goods =
    {
        '蜡烛',
        '火把',
        '金创药（小）',
        '魔法药（小）',
        '金创药（中）',
        '魔法药（中）',
        '地牢逃脱卷',
        '护身符（小）',
    },

    -- legacy offers no trade here
    trade = false,

    buyText =
    {
        '虽然贵了点，但是以后连这都不会有了。',
    },

    topics =
    {
        -- legacy @Ghltod
        {
            id    = 'npc_ghltod',
            label = '购买',
            text  = {'……'},   -- legacy section carries no prose
        },
    },
}
