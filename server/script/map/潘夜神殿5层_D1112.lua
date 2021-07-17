local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿5层_D1112
{
    {
        name = '潘夜云魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 54, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜冰魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 54, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 65, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 65, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜战士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 54, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜火魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 54, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜风魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 54, time = 600, cratio = 0},
        }
    },
    {
        name = '潘夜鬼将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
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
