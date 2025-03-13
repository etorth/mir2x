local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 祖玛神殿4层_D504
{
    {
        name = '大老鼠',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 115, time = 600},
        }
    },
    {
        name = '护法天',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '楔蛾',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 100, time = 600},
        }
    },
    {
        name = '祖玛卫士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 125, time = 600},
        }
    },
    {
        name = '祖玛卫士0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '祖玛卫士8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '祖玛弓箭手',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 80, time = 600},
        }
    },
    {
        name = '祖玛弓箭手0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '祖玛弓箭手8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 1, time = 3600},
        }
    },
    {
        name = '祖玛雕像',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 100, time = 600},
        }
    },
    {
        name = '祖玛雕像0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600},
        }
    },
    {
        name = '祖玛雕像8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600},
        }
    },
    {
        name = '角蝇',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 50, time = 600},
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
