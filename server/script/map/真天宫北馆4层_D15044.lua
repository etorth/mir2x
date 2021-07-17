local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫北馆4层_D15044
{
    {
        name = '地牢女神1',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 120, time = 600, cratio = 0},
        }
    },
    {
        name = '地牢女神2',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 90, time = 600, cratio = 0},
        }
    },
    {
        name = '地牢女神8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 9, time = 600, cratio = 0},
        }
    },
    {
        name = '武力神将',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 120, time = 600, cratio = 0},
        }
    },
    {
        name = '武力神将8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 12, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 25, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 2, time = 600, cratio = 0},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 50, time = 600, cratio = 0},
        }
    },
    {
        name = '石像狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 5, time = 600, cratio = 0},
        }
    },
    {
        name = '震天首将',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
