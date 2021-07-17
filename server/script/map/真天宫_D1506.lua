local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫_D1506
{
    {
        name = '守卫狮子',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 10, time = 300, cratio = 0},
        }
    },
    {
        name = '武力神将',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 5, time = 480, cratio = 0},
        }
    },
    {
        name = '武力神将81',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 5, time = 480, cratio = 0},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 22, y = 40, w = 5, h = 5, count = 5, time = 240, cratio = 0},
            {x = 30, y = 50, w = 20, h = 20, count = 10, time = 480, cratio = 0},
        }
    },
    {
        name = '火焰狮子97',
        loc = {
            {x = 22, y = 40, w = 5, h = 5, count = 5, time = 240, cratio = 0},
        }
    },
    {
        name = '火焰狮子99',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 10, time = 480, cratio = 0},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 22, y = 40, w = 5, h = 5, count = 5, time = 240, cratio = 0},
            {x = 30, y = 50, w = 20, h = 20, count = 10, time = 480, cratio = 0},
        }
    },
    {
        name = '石像狮子96',
        loc = {
            {x = 22, y = 40, w = 5, h = 5, count = 5, time = 240, cratio = 0},
        }
    },
    {
        name = '石像狮子98',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 10, time = 480, cratio = 0},
        }
    },
    {
        name = '震天魔神',
        loc = {
            {x = 30, y = 50, w = 20, h = 20, count = 1, time = 18000, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
