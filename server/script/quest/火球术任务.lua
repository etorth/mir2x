-- converted from Envir/QuestDiary/MU_wizard/fireBolt.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'银杏山谷_02', '霹雳尊者_1'},
    job     = '法师',
    level   = 7,
    book    = '火球术',

    lore =
    {
        '有了火球术魔法书我就可以教你魔法。',
        '想要学习火球术的样子。但是像你一样的初学者，在学习武功的过程中将遇到各种困难，我将给你进行详细地说明。现在你已经正式进入了成为魔法师的大门，恭喜你！',
        '火球术是魔法师的最基本魔法， 制作火团 攻击远处的敌人。',
    },

    reward = '那么在给你秘籍之前，想听对武功的简单说明吗？',
}
