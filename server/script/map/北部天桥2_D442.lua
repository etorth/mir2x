local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部天桥2_D442
{
    {
        name = '僵尸1',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
