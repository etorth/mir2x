local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 暗黑封印_B139_001
{
    {
        name = '祖玛教主62',
        loc = {
            {x = 19, y = 24, w = 1, h = 1, count = 1, time = 300},
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
