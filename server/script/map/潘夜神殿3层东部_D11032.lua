local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿3层东部_D11032
{
    {
        name = '潘夜云魔',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 13, time = 600},
        }
    },
    {
        name = '潘夜冰魔',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 13, time = 600},
        }
    },
    {
        name = '潘夜战士',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 13, time = 600},
        }
    },
    {
        name = '潘夜火魔',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 13, time = 600},
        }
    },
    {
        name = '潘夜风魔',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 13, time = 600},
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
