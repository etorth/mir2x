-- converted from Envir/QuestDiary/MU_taoist/massheal.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'道馆_1', '大悲善僧_1'},
    job     = '道士',
    level   = 31,
    book    = '群体治愈术',

    fetch =
    {
        item    = '威魂深怨护身符',
        monster = {'蜈蚣'},
        kills   = 5,
        say     = '（找到了威魂深怨护身符……）',
    },

    lore =
    {
        '在那个地方该见到谁了嘛。',
        '事情都结束了，就回到我这儿吧',
        '还没有离开那个村庄哟。',
        '那个村庄位于 盟重县东北方向绝命谷入口的附近 。',
        '你已经练成了群体治愈术，我再没有什么魔法可以教你了，以后再来找我吧。',
    },

    reward = '入口附近的某个村子，上 香 即可。不知道那个地方将要发生什么事情，剩余的事情你要自己解决。如果所有的问题都解决了，在重新找我来。',
    gold   = 33000,
    items  = {'神圣铂金戒指'},
}
