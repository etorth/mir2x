local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 长官食堂_D9032
{
    {
        name = '神舰守卫',
        loc = {
            {x = 15, y = 15, w = 15, h = 15, count = 1, time = 1200},
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
