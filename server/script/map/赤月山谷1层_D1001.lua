local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 赤月山谷1层_D1001
{
    {
        name = '灰血恶魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 80, time = 600},
        }
    },
    {
        name = '灰血恶魔0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600},
        }
    },
    {
        name = '神鬼王',
        loc = {
            {x = 200, y = 200, w = 150, h = 150, count = 1, time = 7200},
        }
    },
    {
        name = '血巨人',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 90, time = 600},
        }
    },
    {
        name = '血巨人0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '血金刚',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 85, time = 600},
        }
    },
    {
        name = '血金刚0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600},
        }
    },
    {
        name = '赤血恶魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 80, time = 600},
        }
    },
    {
        name = '赤血恶魔0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
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
