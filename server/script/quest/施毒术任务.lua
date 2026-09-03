-- converted from Envir/QuestDiary/MU_taoist/amyen.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 12,
    book    = '施毒术',

    fetch =
    {
        item    = '蛆卵',
    },

    trial =
    {
        map      = '试练场_1_008',
        x        = 5,
        y        = 14,
        minutes  = 5,
        monsters = {{'毒蜘蛛', 3}, {'食人花', 3}, {'蝎子', 3}, {'洞蛆', 3}},
        exit     = {'本馆_1_002', 11, 11},
    },

    lore =
    {
        '首先对毒粉进行说明。毒粉包括 黄色毒粉 和  灰色毒粉 。对这些材料 药剂师 比我更清楚，请问他们！',
        '你在学习施毒术之前，首先要掌握材料的毒性。现在我送你去某个地方， 直接采取材料 进行学习。采取的方法当作像切肉一样的熟练工种即可。',
        '时间是5分钟。。',
        '虽然学习、应用施毒术有些复杂，施毒术是道士的 唯一进攻辅助魔法 ，它的效果非常高。',
        '现在虽然困难，在最短的时间内掌握施毒术还是要好些。',
    },

    reward = '辛苦了！给你武功秘籍，剩余的部分自己掌握吧。',
    gold   = 14000,
    items  = {'天仙之珠'},
}
