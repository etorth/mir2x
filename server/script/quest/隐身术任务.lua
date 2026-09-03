-- converted from Envir/QuestDiary/MU_taoist/hiden.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 20,
    book    = '隐身术',

    fetch =
    {
        item    = '蜡烛',
    },

    trial =
    {
        map      = '试练场_1_010',
        x        = 40,
        y        = 64,
        minutes  = 5,
        monsters = {{'沃玛勇士', 10}, {'雷电僵尸', 10}, {'沃玛护卫', 10}},
        exit     = {'本馆_1_002', 11, 11},
    },

    lore =
    {
        '在如此黑暗之中，好象还没有适应的样子。',
        '不要勉强解决问题，保持一个平常心，多试几次终究会成功的。',
        '想知道叫做隐身术的武功吧？',
        '隐身术是 使怪兽们无法发现自己行踪，从而隐藏自己行踪的魔法 。首先不动弹，不被发现。在危急的时候就会有很大的帮助。',
        '为了学习隐身术要领会隐藏自己痕迹的方法，因此要到特殊的训练场累积些经验。',
    },

    reward = '我给你隐身术秘籍，剩下的部分你自己修炼吧。',
    gold   = 20000,
    items  = {'暗黑凤凰明珠'},
}
