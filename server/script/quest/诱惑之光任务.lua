-- converted from Envir/QuestDiary/MU_wizard/lightShock.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'银杏山谷_02', '霹雳尊者_1'},
    job     = '法师',
    level   = 13,
    book    = '诱惑之光',

    trial =
    {
        map      = '试练场_02_001',
        x        = 9,
        y        = 12,
        minutes  = 2,
        monsters = {{'半兽人', 5}, {'骷髅', 5}},
        exit     = {'银杏山谷_02', 265, 146},
    },

    lore =
    {
        '还没有给你诱惑之光的解析吗？',
        '诱惑之光是一种瞬间里 发射威力强大的闪电，使得怪物们恐慌的魔法 。如果使用得当，一时间怪物们都无法行动。尤其是可以 控制比你能力低很多怪物们精神的可怕魔法 。',
        '你还没有掌握称为诱惑之光的魔法吗？',
        '你还没有达到修炼诱惑之光的等级。。请继续修炼，达到13级为止。',
        '你还不是魔法师吗？如果不是魔法师，还无法修炼该武功。',
    },

    reward = '如果想学诱惑之光，在一定时间之内将考场内的怪物们都制服即可。',
    gold   = 13000,
    items  = {'魔家项链'},
}
