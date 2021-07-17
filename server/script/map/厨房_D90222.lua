local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 厨房_D90222
{
    {
        name = '触角神魔',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 1, time = 1200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
