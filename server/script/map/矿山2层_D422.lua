local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 矿山2层_D422
{
    {
        name = '僵尸1',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600},
        }
    },
    {
        name = '僵尸10',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 2, time = 3600},
        }
    },
    {
        name = '僧侣僵尸',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600},
        }
    },
    {
        name = '僵尸30',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 2, time = 3600},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600},
        }
    },
    {
        name = '僵尸40',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 2, time = 3600},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600},
        }
    },
    {
        name = '尸王',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
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
