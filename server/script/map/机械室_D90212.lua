local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 机械室_D90212
{
    {
        name = '恶形鬼',
        loc = {
            {x = 13, y = 13, w = 13, h = 13, count = 1, time = 1200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
