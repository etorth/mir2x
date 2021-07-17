local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 西部石矿1层_D1421
{
    {
        name = '僵尸1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸10',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸20',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '洞蛆',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
