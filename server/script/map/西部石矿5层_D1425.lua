local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 西部石矿5层_D1425
{
    {
        name = '僵尸1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸40',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸50',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '尸王',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
