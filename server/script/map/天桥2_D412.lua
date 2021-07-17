local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 天桥2_D412
{
    {
        name = '僵尸1',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
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
