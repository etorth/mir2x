local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿2层_D1102
{
    {
        name = '潘夜云魔',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 45, time = 600},
        }
    },
    {
        name = '潘夜冰魔',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 45, time = 600},
        }
    },
    {
        name = '潘夜冰魔0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600},
        }
    },
    {
        name = '潘夜冰魔8',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600},
        }
    },
    {
        name = '潘夜战士',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 45, time = 600},
        }
    },
    {
        name = '潘夜火魔',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 45, time = 600},
        }
    },
    {
        name = '潘夜火魔0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600},
        }
    },
    {
        name = '潘夜火魔8',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600},
        }
    },
    {
        name = '潘夜风魔',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 45, time = 600},
        }
    },
    {
        name = '潘夜鬼将',
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
