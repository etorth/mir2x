local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 神圣战甲术空间_D400_001
{
    {
        name = '尸王61',
        loc = {
            {x = 25, y = 25, w = 25, h = 25, count = 10, time = 60, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
