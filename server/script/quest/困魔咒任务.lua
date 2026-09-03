-- converted from Envir/QuestDiary/MU_taoist/holy.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'道馆_1', '大悲善僧_1'},
    job     = '道士',
    level   = 27,
    book    = '困魔咒',

    fetch =
    {
        item    = '最后困魔石',
        monster = {'祖玛卫士'},
        kills   = 5,
        say     = '（找到了最后困魔石……）',
    },

    lore =
    {
        '托你的福，那个地方的 困魔咒被完好地修复了 。你去过后加强了那个地方的警卫，以使困魔咒不再受到损伤。',
        '因此没有认为是一件简单的事情，怪兽们的抵抗力是如此的强大。',
        '虽然如此也不是就这样可以放弃的事情。',
        '困魔咒的房间在 沃玛神殿2层里面 。',
        '迅速将困魔咒复原。',
    },

    reward = '处理了最后房间怪兽的头儿，请重新找我来。',
}
