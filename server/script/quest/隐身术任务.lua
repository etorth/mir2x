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
        say     = '（找到了蜡烛……）',
        desp    = '去取得蜡烛，再回去找清明子修炼隐身术。',
    },

    lore =
    {
        '在如此黑暗之中，好象还没有适应的样子。',
        '不要勉强解决问题，保持一个平常心，多试几次终究会成功的。',
        '噢噢，现在平静一下心情，想 重新挑战一次 吗？你这次一定可以成功。',
        '想知道叫做隐身术的武功吧？',
        '隐身术是 使怪兽们无法发现自己行踪，从而隐藏自己行踪的魔法 。首先不动弹，不被发现。在危急的时候就会有很大的帮助。',
        '为了学习隐身术要领会隐藏自己痕迹的方法，因此要到特殊的训练场累积些经验。',
    },

    reward = '训练场里的怪兽们蠕动着，要尽量回避回到这里。',
}
