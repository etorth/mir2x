local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 石阁1层_D711
{
    {
        name = '白野猪',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '红野猪',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 320, time = 600, cratio = 0},
        }
    },
    {
        name = '红野猪0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 3, time = 3600, cratio = 0},
        }
    },
    {
        name = '黑野猪',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 300, time = 600, cratio = 0},
        }
    },
    {
        name = '黑野猪0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 3, time = 3600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
