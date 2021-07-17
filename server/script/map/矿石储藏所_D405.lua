local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 矿石储藏所_D405
{
    {
        name = '僧侣僵尸',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 4, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸1',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸20',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸40',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸50',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '尸王',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '雷电僵尸',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 4, time = 600, cratio = 0},
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
