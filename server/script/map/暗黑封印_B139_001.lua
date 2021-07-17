local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 暗黑封印_B139_001
{
    {
        name = '祖玛教主62',
        loc = {
            {x = 19, y = 24, w = 1, h = 1, count = 1, time = 300, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
