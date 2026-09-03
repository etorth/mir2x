-- converted from Envir/QuestDiary/MU_taoist/ilgang.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'本馆_1_002', '清明子_1'},
    job     = '道士',
    level   = 8,
    book    = '精神力战法',

    trial =
    {
        map      = '试练场_1_012',
        x        = 9,
        y        = 12,
        minutes  = 5,
        monsters = {{'半兽战士', 1}, {'半兽人', 3}},
        exit     = {'本馆_1_002', 11, 11},
    },

    lore =
    {
        '不是不管三七二十一就舞剑。 先走心剑紧随其后。 保持心如止水，冷静对敌。',
        '想重新接受修炼吗？',
        '精神力战法是剑术造诣很深的某个先辈故人创造的 为了道士的剑法 。道士们终究是比战士们力量弱，如果不学习精神力战法，放弃 直接进攻 还是好些。',
        '现在你已经到了该修炼精神力战法的时候，我教你修炼。和修炼其它的魔法不一样，现在是修炼剑法，所以修炼方法和战士的修炼方法没有什么不同的。',
        '怎么样？接受修炼吗？',
    },

    reward = '这个是精神力战法要点解释的 秘籍 ，请拿走看看。你已经具有了基本素质，只要掌握要点，充分地可以学习精神力战法。',
    gold   = 9000,
    items  = {'灵魂铁手镯'},
}
