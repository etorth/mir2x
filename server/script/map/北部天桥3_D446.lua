local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部天桥3_D446
{
    {
        name = '僵尸1',
        loc = {
            {x = 0, y = 0, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '僧侣僵尸',
        loc = {
            {x = 0, y = 0, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 0, y = 0, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 0, y = 0, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 0, y = 0, w = 100, h = 100, count = 20, time = 600},
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
