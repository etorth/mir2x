-- converted from Envir/Market_Def/15Magic_DoGwan-1_002.txt
-- fees from Envir/QuestDiary/MU_Total_Sell/Taoist

local skillteacher = require('npc.include.skillteacher')

skillteacher.setTeacher
{
    job   = '道士',
    intro = '贫道就是清明子，专门在这里指导那些想修炼成为道士的人。',
    greet = '贫道就是清明子。',
    ask   = '那，你来找我有什么事？',

    redName = '跟你这种人我无话可说。',
    wrongJob =
    {
        ['战士'] = '不过你是战士，你还是去边境城市吧。',
        ['法师'] = '不过你是魔法师，你还是去银杏山谷吧。',
    },

    books =
    {
        {
            band = '1 - 10 等级 修炼魔法',
            list =
            {
                {'治愈术',     700},
                {'精神力战法', 800},
            },
        },

        {
            band = '11 - 25 等级 修炼魔法',
            list =
            {
                {'施毒术',      1200},
                {'召唤骷髅',    1700},
                {'隐身术',      2000},
                {'集体隐身术',  2300},
                {'神圣战甲术',  2500},
            },
        },
    },
}
