local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 行会大战_E001 TODO
{
    {
        name = '山洞蝙蝠',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380, cratio = 50},
        }
    },
    {
        name = '掷斧骷髅',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380, cratio = 0},
        }
    },
    {
        name = '洞蛆',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380, cratio = 0},
        }
    },
    {
        name = '骷髅',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 15, time = 1380, cratio = 0},
        }
    },
    {
        name = '骷髅战士',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 1380, cratio = 0},
        }
    },
    {
        name = '骷髅战将',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 1380, cratio = 0},
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
