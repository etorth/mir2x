local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 南部矿山_D431 TODO
{
    {
        name = '僧侣僵尸',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸1',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸10',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸30',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
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
        name = '洞蛆',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 33, time = 600, cratio = 0},
        }
    },
    {
        name = '雷电僵尸',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 10, time = 600, cratio = 0},
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
