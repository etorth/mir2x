local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫南馆3层_D15032
{
    {
        name = '地牢女神1',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 90, time = 600},
        }
    },
    {
        name = '地牢女神2',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 90, time = 600},
        }
    },
    {
        name = '地牢女神8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 9, time = 600},
        }
    },
    {
        name = '武力神将',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 60, time = 600},
        }
    },
    {
        name = '武力神将8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 6, time = 600},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 115, time = 600},
        }
    },
    {
        name = '火焰狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 15, time = 600},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 25, time = 600},
        }
    },
    {
        name = '石像狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 2, time = 600},
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
