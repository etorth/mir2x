-- converted from Envir/QuestDiary/MU_wizard/fireUpbolt.txt

local skillquest = require('quest.include.skillquest')

skillquest.setSkillQuest
{
    teacher = {'银杏山谷_02', '霹雳尊者_1'},
    job     = '法师',
    level   = 15,
    book    = '大火球',

    lore =
    {
        '你不是已经收到大火球秘籍了吗？ 那么你为什么还要索要？',
        '大火球是 强化了的火球术 。正如它的名字一样是可以放出将金刚石熔化 强大火团的技法 。如果掌握了第2阶段的火球，进一步修炼大火球还是比较好。',
        '但是你现在好像还没有到可以学习的时候。做好学习准备时，请再来！',
        '想修炼大火球魔法吗？',
        '你还没有掌握大火球魔法吗？',
        '嘿嘿，知道了。这样的话，我就告诉你 修炼大火球的方法 。大火球是将 强大的火团射向敌人的魔法 ，除去威力比较大之外，同火球没有很大的差异。',
    },

    reward = '为了通过测试一定要佩戴焱火剑，如果丢失了，请花钱买！',
}
