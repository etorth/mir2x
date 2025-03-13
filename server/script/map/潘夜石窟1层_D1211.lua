local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜石窟1层_D1211
{
    {
        name = '骨鬼将',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200},
        }
    },
    {
        name = '骷髅士兵',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '骷髅士兵0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '骷髅弓箭手',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 80, time = 600},
        }
    },
    {
        name = '骷髅弓箭手0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '骷髅武士',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 80, time = 600},
        }
    },
    {
        name = '骷髅武将',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 70, time = 600},
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
