local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 灌木林_12
{
    {
        name = '八脚首领',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 100, time = 1200, cratio = 0},
        }
    },
    {
        name = '天狼蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 154, time = 600, cratio = 0},
        }
    },
    {
        name = '天狼蜘蛛0',
        loc = {
            {x = 66, y = 66, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
            {x = 332, y = 66, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '幻影蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '月魔蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 154, time = 600, cratio = 0},
        }
    },
    {
        name = '独眼蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 154, time = 600, cratio = 0},
        }
    },
    {
        name = '独眼蜘蛛0',
        loc = {
            {x = 199, y = 332, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '花色蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 154, time = 600, cratio = 0},
        }
    },
    {
        name = '花色蜘蛛0',
        loc = {
            {x = 66, y = 332, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
            {x = 332, y = 332, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '黑角蜘蛛',
        loc = {
            {x = 200, y = 250, w = 200, h = 200, count = 154, time = 600, cratio = 0},
        }
    },
    {
        name = '黑角蜘蛛0',
        loc = {
            {x = 199, y = 66, w = 66, h = 66, count = 1, time = 3600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
