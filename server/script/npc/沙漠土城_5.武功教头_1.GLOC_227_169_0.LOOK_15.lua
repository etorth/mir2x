-- converted from Envir/Market_Def/15Magic_Samak1-5.txt
-- fees from Envir/QuestDiary/MU_Total_Sell

local skillteacher = require('npc.include.skillteacher')

skillteacher.setTeacher
{
    greet = '很高兴见到这样英姿飒爽的勇士，本教头是奉比奇朝廷的命令来这里帮助那些穿越沙漠的勇士修炼武功的。原本武林有一条不成文的规定，就是朝廷不干涉像你这样的江湖武士的活动，但在这种危急的时候谁还管那么多呢？啊，废话说多了。你新拿到的武功书里有什么看不懂的地方都可以问我，不过要支付一些费用才行。',
    redName = '跟你这种人我无话可说。',

    books =
    {
        ['战士'] =
        {
            {
                band = '1 - 10 等级 修炼魔法',
                list =
                {
                    {'基本剑术', 700},
                },
            },

            {
                band = '11 - 20 等级 修炼魔法',
                list =
                {
                    {'攻杀剑术', 1400},
                    {'刺杀剑术', 1900},
                },
            },

            {
                band = '21 - 30 等级 修炼魔法',
                list =
                {
                    {'半月弯刀', 2300},
                    {'野蛮冲撞', 2700},
                },
            },

            {
                band = '31 - 40 等级 修炼魔法',
                list =
                {
                    {'烈火剑法', 3200},
                    {'翔空剑法', 3500},
                    {'莲月剑法', 3800},
                },
            },

        },
    },
}
