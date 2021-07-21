local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沙漠_76
{
    {
        name = '多角虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 200, y = 100, w = 100, h = 100, count = 75, time = 1200},
        }
    },
    {
        name = '猎鹰0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '盔甲虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600},
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
