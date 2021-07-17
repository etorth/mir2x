local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿入口_D021
{
    {
        name = '山洞蝙蝠',
        loc = {
            {x = 50, y = 50, w = 60, h = 60, count = 25, time = 600, cratio = 0},
        }
    },
    {
        name = '洞蛆',
        loc = {
            {x = 50, y = 50, w = 60, h = 60, count = 25, time = 600, cratio = 0},
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
