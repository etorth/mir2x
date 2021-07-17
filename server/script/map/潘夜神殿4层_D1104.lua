local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿4层_D1104
{
    {
        name = '潘夜云魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜冰魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫0',
        loc = {
            {x = 100, y = 100, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫8',
        loc = {
            {x = 100, y = 100, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫0',
        loc = {
            {x = 100, y = 100, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫8',
        loc = {
            {x = 100, y = 100, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜战士',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜火魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜风魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 28, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜鬼将',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200, cratio = 0},
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
