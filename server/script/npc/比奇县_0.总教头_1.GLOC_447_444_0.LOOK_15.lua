-- converted from Envir/Market_Def/15Magic_Bichon1-0.txt
-- fees from Envir/QuestDiary/MU_Total_Sell

local skillteacher = require('npc.include.skillteacher')

skillteacher.setTeacher
{
    greet = '我一看就知道你是战士。怎么样，一个人修炼武功有什么困难吗？好不容易弄来的秘籍，却由于无法理解而导致修炼中出现差错，如果那样的话，我可以帮助你。不过，不管怎么说我也是军队里的武功教练，所以你得交点钱我才能指导你。',
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

            {
                band = '41 - 50 等级 修炼魔法',
                list =
                {
                    {'十方斩', 10000},
                    {'乾坤大挪移', 10000},
                    {'铁布衫', 10000},
                    {'斗转星移', 10000},
                    {'破血狂杀', 10000},
                },
            },

        },
    },
}
