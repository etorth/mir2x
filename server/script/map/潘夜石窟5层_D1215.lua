local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜石窟5层_D1215
{
    {
        name = '守卫武将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 40, time = 300},
        }
    },
    {
        name = '骨鬼将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '骷髅士兵',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 200, time = 600},
        }
    },
    {
        name = '骷髅士兵0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '骷髅弓箭手',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 180, time = 600},
        }
    },
    {
        name = '骷髅弓箭手0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '骷髅教主',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 18000},
        }
    },
    {
        name = '骷髅武士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 180, time = 600},
        }
    },
    {
        name = '骷髅武将',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 200, time = 600},
        }
    },
    {
        name = '骷髅武将0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
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
