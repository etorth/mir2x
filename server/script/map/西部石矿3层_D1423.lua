local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 西部石矿3层_D1423
{
    {
        name = '僵尸1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '僵尸10',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
        }
    },
    {
        name = '僧侣僵尸',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '僵尸50',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
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
