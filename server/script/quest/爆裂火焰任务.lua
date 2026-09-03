-- converted from Envir/QuestDiary/MU_wizard/pokyel.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'银杏山谷_02', '化天先生_1'},
    job     = '法师',
    level   = 32,
    book    = '爆裂火焰',

    fetch =
    {
        item    = '七点白蛇胆',
        monster = {'七点白蛇'},
        kills   = 4,
        say     = '（找到了七点白蛇胆……）',
    },

    lore =
    {
        '快点到毒蛇山村寻找七点白蛇的胆汁来。',
        '你有什么事吗？说说看。。',
        '嗯，想学称为“爆裂火焰”的武功？',
        '如果需要帮忙，请随时来找我！',
        '嗯。。你现在学习该武功还是有些早。提高武功等级后再来吧！',
    },

    reward = '喂，这里有药水。这个药水是用你拿来的 胆汁制成的 。你吃药的过程中，我将准备武功秘籍。',
    gold   = 99000,
    items  = {'流星天玉'},
}
