local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿1层_D032
{
    {
        name = '山洞蝙蝠',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 52, time = 600, cratio = 0},
        }
    },
    {
        name = '山洞蝙蝠0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛勇士',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '沃玛勇士0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛勇士60',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 2, time = 7200, cratio = 0},
        }
    },
    {
        name = '沃玛卫士',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 2, time = 7200, cratio = 0},
        }
    },
    {
        name = '沃玛战士',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '沃玛战士0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '沃玛战将',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰沃玛',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '火焰沃玛0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '粪虫',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 60, time = 600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
