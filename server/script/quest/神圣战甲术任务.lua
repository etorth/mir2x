-- converted from Envir/QuestDiary/MU_taoist/deaji.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 25,
    book    = '神圣战甲术',

    fetch =
    {
        item    = '起爆石',
        monster = {'尸王'},
        kills   = 5,
        say     = '（找到了起爆石……）',
    },

    lore =
    {
        '你还没有能力修炼神圣战甲术..到了 等级 25 再来找我吧。',
        '使用神圣战甲术可以瞬间吸收大自然的真气， 提高物理防御力 。',
        '你想修炼神圣战甲术吗?要想修炼神圣战甲术就要学会吸收大自然真气的方法。世上万物各有各的真气，神圣战甲术就是吸收这种真气， 一定时间内保护自己 。',
        '前面我也说了，要想修炼神圣战甲术就要学会吸收 大自然真气 的方法，你还没有这个能力，小心走火入魔啊。',
        '嗯....为了修炼新的技术而变换自己或许是可怕的事情。做好心理准备之后再来吧。',
    },

    reward = '把起爆石给我带来即可。',
    gold   = 25000,
    items  = {'八面太极戒指'},
    genderItems = {male = '神奇灵魂战衣（男）', female = '神奇灵魂战衣（女）'},
}
