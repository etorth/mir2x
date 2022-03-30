local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 石阁6层_D716
{
    {
        name = '楔蛾',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
        }
    },
    {
        name = '红野猪',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
        }
    },
    {
        name = '蝎蛇',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
        }
    },
    {
        name = '角蝇',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
        }
    },
    {
        name = '黑野猪',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 600},
        }
    },
})

function main()
    while true do
        local rc, errMsg = coroutine.resume(addMonCo)
        if not rc then
            fatalPrintf('addMonCo failed: %s', argDefault(errMsg, 'unknown error'))
        end
        asyncWait(1000 * 5)
    end
end
