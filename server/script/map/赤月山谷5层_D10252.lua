local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 赤月山谷5层_D10252
{
    {
        name = '灰血恶魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600},
        }
    },
    {
        name = '灰血恶魔0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 2700},
        }
    },
    {
        name = '神鬼王',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 7200},
        }
    },
    {
        name = '血金刚',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 50, time = 600},
            {x = 150, y = 150, w = 150, h = 150, count = 50, time = 600},
        }
    },
    {
        name = '血金刚0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 2700},
        }
    },
    {
        name = '赤血恶魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600},
        }
    },
    {
        name = '赤血恶魔0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 2, time = 2700},
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
