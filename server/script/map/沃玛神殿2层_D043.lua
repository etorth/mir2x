local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿2层_D043
{
    {
        name = '守卫沃玛',
        loc = {
            {x = 49, y = 362, w = 20, h = 20, count = 20, time = 300},
        }
    },
    {
        name = '暗黑战士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
        }
    },
    {
        name = '沃玛勇士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
        }
    },
    {
        name = '沃玛勇士0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '沃玛勇士60',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 7200},
        }
    },
    {
        name = '沃玛卫士',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '沃玛战士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
        }
    },
    {
        name = '沃玛战士0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '沃玛战将',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
        }
    },
    {
        name = '沃玛战将0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '沃玛护卫',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 10800},
        }
    },
    {
        name = '火焰沃玛',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
        }
    },
    {
        name = '粪虫',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 60, time = 600},
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
