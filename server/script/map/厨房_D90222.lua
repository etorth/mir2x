local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 厨房_D90222
{
    {
        name = '触角神魔',
        loc = {
            {x = 3, y = 15, w = 25, h = 25, count = 1, time = 1200},
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
