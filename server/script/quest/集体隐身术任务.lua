-- converted from Envir/QuestDiary/MU_taoist/masshiden.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 23,
    book    = '集体隐身术',

    fetch =
    {
        item    = '成致日志',
        monster = {'半兽勇士'},
        kills   = 5,
        say     = '（找到了成致日志……）',
        desp    = '去取得成致日志，再回去找清明子修炼集体隐身术。',
    },

    lore =
    {
        '还没有拜见清明子吗？',
        '去了以后问一下叫 小贩 男人的情况。',
        '你看起来是非常有实力的道士哦。你知道有关 小贩 男子的故事吗？',
        '哦，不知道也是理所当然的。这个男人是不久之前逃到比奇省的伪道士，到处讲道士们的坏话。',
        '但是听了他的故事，他也很为难哟。',
        '好的，那么请走好。下次不要忘了多买些东西。',
    },

    reward = '称为痕迹的东西有可能就是他的 日志 。他的日志被那个地方的怪兽拿着的可能性很高。',
}
