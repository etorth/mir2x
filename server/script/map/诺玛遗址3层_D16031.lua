local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 诺玛遗址3层_D16031
{
    {
        name = '诺玛司令',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 110, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛司令8',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 11, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛抛石兵',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 100, time = 600, cratio = 30},
        }
    },
    {
        name = '诺玛抛石兵8',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 10, time = 600, cratio = 30},
        }
    },
    {
        name = '诺玛斧兵',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 80, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛斧兵8',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 8, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛突击队长',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '诺玛装甲兵',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 130, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛装甲兵8',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 13, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛骑兵',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 130, time = 600, cratio = 0},
        }
    },
    {
        name = '诺玛骑兵8',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 13, time = 600, cratio = 0},
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
