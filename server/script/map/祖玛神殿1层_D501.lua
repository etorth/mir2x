local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 祖玛神殿1层_D501
{
    {
        name = '大老鼠',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 225, time = 600, cratio = 0},
        }
    },
    {
        name = '大老鼠0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600, cratio = 0},
        }
    },
    {
        name = '大老鼠8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600, cratio = 0},
        }
    },
    {
        name = '楔蛾',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 150, time = 600, cratio = 0},
        }
    },
    {
        name = '祖玛弓箭手',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 130, time = 600, cratio = 0},
        }
    },
    {
        name = '祖玛弓箭手0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600, cratio = 0},
        }
    },
    {
        name = '祖玛弓箭手8',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 2, time = 3600, cratio = 0},
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
