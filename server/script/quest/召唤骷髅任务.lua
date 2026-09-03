-- converted from Envir/QuestDiary/MU_taoist/soulSkel.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 17,
    book    = '召唤骷髅',

    trial =
    {
        map      = '试练场_1_013',
        x        = 23,
        y        = 25,
        minutes  = 10,
        monsters = {{'骷髅战将', 4}, {'变异骷髅', 1}},
        exit     = {'道馆_1', 350, 402},
    },

    lore =
    {
        '或者把它丢失在在那儿了？',
        '如果没有回应召唤的 守护灵 ，即使明白了召唤骷髅的道理也没有任何作用。',
        '再次去地牢空间吗？',
        '感受到保护你的正气了吧!',
        '你已经修炼了 召唤骷髅 ，也没有必要在接受我的指教了。',
    },

    reward = '你已经在其它地方得到了武功秘籍，我也没有再给你的必要了。如果可以熟练地掌握这本书，以后即使你一个人修炼没有什么问题。',
    gold   = 19000,
    items  = {'幻影玉珠'},
}
