local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 真天宫北馆4层_D15043
{
    {
        name = '地牢女神1',
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
            {x = 150, y = 150, w = 140, h = 140, count = 120, time = 600},
        }
    },
    {
        name = '武力神将8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 12, time = 600},
        }
    },
    {
        name = '火焰狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 95, time = 600},
        }
    },
    {
        name = '火焰狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 9, time = 600},
        }
    },
    {
        name = '石像狮子',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 95, time = 600},
        }
    },
    {
        name = '石像狮子8',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 9, time = 600},
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
            fatalPrintf('addMonCo failed: %s', argDef(errMsg, 'unknown error'))
        end
        asyncWait(1000 * 5)
    end
end
