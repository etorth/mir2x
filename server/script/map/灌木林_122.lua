local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 灌木林_122
{
    {
        name = '八脚首领',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 100, time = 1200},
        }
    },
    {
        name = '天狼蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 154, time = 600},
        }
    },
    {
        name = '天狼蜘蛛0',
        loc = {
            {x = 120, y = 280, w = 80, h = 80, count = 1, time = 3600},
            {x = 280, y = 120, w = 80, h = 80, count = 1, time = 3600},
        }
    },
    {
        name = '幻影蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 65, time = 600},
        }
    },
    {
        name = '月魔蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 154, time = 600},
        }
    },
    {
        name = '独眼蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 154, time = 600},
        }
    },
    {
        name = '独眼蜘蛛0',
        loc = {
            {x = 120, y = 120, w = 80, h = 80, count = 1, time = 3600},
            {x = 280, y = 120, w = 80, h = 80, count = 1, time = 3600},
        }
    },
    {
        name = '花色蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 154, time = 600},
        }
    },
    {
        name = '花色蜘蛛0',
        loc = {
            {x = 120, y = 120, w = 80, h = 80, count = 1, time = 3600},
            {x = 120, y = 280, w = 80, h = 80, count = 1, time = 3600},
            {x = 280, y = 280, w = 80, h = 80, count = 1, time = 3600},
        }
    },
    {
        name = '黑角蜘蛛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 154, time = 600},
        }
    },
    {
        name = '黑角蜘蛛0',
        loc = {
            {x = 280, y = 280, w = 80, h = 80, count = 1, time = 3600},
        }
    },
})

function main()
    while true do
        local rc, errMsg = coroutine.resume(addMonCo)
        if not rc then
            fatalPrintf('addMonCo failed: %s', argDef(errMsg, 'unknown error'))
        end
        asyncWait(1000 * 5)
    end
end
