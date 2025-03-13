local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿1层_D052
{
    {
        name = '沃玛卫士',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 2, time = 7200},
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
