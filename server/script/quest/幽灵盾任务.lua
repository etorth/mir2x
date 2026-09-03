-- converted from Envir/QuestDiary/MU_taoist/hangma.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 21,
    book    = '幽灵盾',

    fetch =
    {
        item    = '灵珠',
        monster = {'骷髅精灵'},
        kills   = 5,
        say     = '（找到了灵珠……）',
        desp    = '去取得灵珠，再回去找清明子修炼幽灵盾。',
    },

    lore =
    {
        '在做什么。。不到飞天废矿找 ‘灵珠’  ，认为现在是可以磨磨噌噌的时候嘛？现在很多人正在死去。千万快些 ！！',
        '有什么事情找我吗？',
        '很高兴，我就是大飞圣僧。',
        '对不起，施主不是道士，不能修炼该武功，请回去吧！',
        '嗯，虽然不可以，那时还不具备修炼该武功的能力。后会有期！',
        '嘿嘿，虽然不知道什么事情，如果我可以帮忙就好了。',
    },

    reward = '若想学幽灵盾，从魔法僵尸那儿找回 灵珠 即可。',
}
