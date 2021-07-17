local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 赤月恶魔洞穴_D10162
{
    {
        name = '守卫血魔',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 5, time = 300, cratio = 0},
        }
    },
    {
        name = '灰血恶魔',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '灰血恶魔60',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '血金刚',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '血金刚70',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '赤月恶魔',
        loc = {
            {x = 23, y = 18, w = 0, h = 0, count = 1, time = 18000, cratio = 0},
        }
    },
    {
        name = '赤血恶魔',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '赤血恶魔50',
        loc = {
            {x = 17, y = 21, w = 17, h = 17, count = 3, time = 3600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
