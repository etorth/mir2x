local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿4层_D1111
{
    {
        name = '潘夜云魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜冰魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 50, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫8',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 50, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜战士',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜火魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜火魔0',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜火魔8',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜风魔',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜鬼将',
        loc = {
            {x = 150, y = 150, w = 140, h = 140, count = 1, time = 7200, cratio = 0},
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
