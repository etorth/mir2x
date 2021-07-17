local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫地下1层_D1500
{
    {
        name = '地牢女神1',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 100, time = 600, cratio = 0},
        }
    },
    {
        name = '地牢女神8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 135, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰狮子8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 13, time = 600, cratio = 0},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 125, time = 600, cratio = 0},
        }
    },
    {
        name = '石像狮子8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 12, time = 600, cratio = 0},
        }
    },
    {
        name = '震天首将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
