local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫北馆5层_D15054
{
    {
        name = '地牢女神1',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 50, time = 600},
        }
    },
    {
        name = '地牢女神2',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 50, time = 600},
        }
    },
    {
        name = '地牢女神8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 50, time = 600},
        }
    },
    {
        name = '守卫狮子',
        loc = {
            {x = 290, y = 293, w = 5, h = 5, count = 3, time = 300},
            {x = 298, y = 285, w = 5, h = 5, count = 3, time = 300},
            {x = 284, y = 298, w = 5, h = 5, count = 3, time = 300},
        }
    },
    {
        name = '武力神将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 190, time = 600},
        }
    },
    {
        name = '武力神将8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 10, time = 600},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 25, time = 600},
        }
    },
    {
        name = '火焰狮子8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 2, time = 600},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 175, time = 600},
        }
    },
    {
        name = '石像狮子8',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 15, time = 600},
        }
    },
    {
        name = '震天首将',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 1, time = 7200},
        }
    },
})

function main()
    while true do
        local rc, errMsg = coroutine.resume(addMonCo)
        if not rc then
            fatalPrintf('addMonCo failed: %s', argDefault(errMsg, 'unknown error'))
        end
        asyncWait(1000 * 5)
    end
end
