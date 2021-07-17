local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿_D024
{
    {
        name = '守卫沃玛',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '暗黑战士',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '暗黑战士40',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛勇士',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛勇士10',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛战将',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛战将20',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛教主',
        loc = {
            {x = 50, y = 50, w = 40, h = 40, count = 1, time = 18000, cratio = 0},
        }
    },
    {
        name = '火焰沃玛',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
    {
        name = '火焰沃玛30',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 25, time = 3600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
