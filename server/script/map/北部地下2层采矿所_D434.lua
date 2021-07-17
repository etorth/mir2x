local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部地下2层采矿所_D434
{
    {
        name = '僵尸1',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸10',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸2',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸20',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸40',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '僵尸50',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '尸王',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
