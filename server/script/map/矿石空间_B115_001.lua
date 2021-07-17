local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 矿石空间_B115_001
{
    {
        name = '僵尸1',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 20, time = 300, cratio = 0},
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
