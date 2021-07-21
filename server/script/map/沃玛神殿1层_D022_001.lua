local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿1层_D022_001
{
    {
        name = '沃玛战将61',
        loc = {
            {x = 57, y = 30, w = 7, h = 7, count = 20, time = 180, cratio = 100},
            {x = 49, y = 43, w = 40, h = 40, count = 40, time = 180},
        }
    },
    {
        name = '火焰沃玛61',
        loc = {
            {x = 57, y = 30, w = 7, h = 7, count = 10, time = 180, cratio = 100},
            {x = 48, y = 33, w = 40, h = 40, count = 15, time = 180},
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
