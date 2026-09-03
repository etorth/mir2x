-- converted from Envir/QuestDiary/MU_wizard/fireStorm.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'银杏山谷_02', '霹雳尊者_1'},
    job     = '法师',
    level   = 12,
    book    = '抗拒火环',

    trial =
    {
        map      = '试练场_02_002',
        x        = 40,
        y        = 64,
        minutes  = 5,
        monsters = {{'半兽人', 10}, {'毒蜘蛛', 10}, {'半兽战士', 5}},
        exit     = {'银杏山谷_02', 265, 145},
    },

    lore =
    {
        '想知道叫“抗拒火环“的武功吗？',
        '抗拒火环是一种被敌人包围时，在自己周围产生 强烈的火墙 ，从而逃脱包围的魔法。也是体力弱魔术师必须掌握的魔法。',
        '但是仅凭语言是无法理解的，只用直接被敌人包围，并体验生命受到威胁才可以学会的。但是这种方法太粗糙。。。要试一下吗？',
        '知道了。那么告诉你方法。现在我把你送到怪物出没的地方。',
        '我站在房间的另一侧，无论有任何事情 都不能干扰怪物或者杀死怪物，只能向我跑过来 。',
    },

    reward = '我将站在终点，你将重新回到这里。需要注意的是 不能伤害考场内的任何一头怪物',
    gold   = 12000,
    items  = {'风之黑檀项链'},
}
