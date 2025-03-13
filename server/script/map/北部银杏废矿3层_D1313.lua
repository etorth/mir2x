local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部银杏废矿3层_D1313
{
    {
        name = '多脚虫1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600},
        }
    },
    {
        name = '多脚虫2',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600},
        }
    },
    {
        name = '红甲虫',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '胞眼虫1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600},
        }
    },
    {
        name = '胞眼虫2',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600},
        }
    },
    {
        name = '蜘蛛娃',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 55, time = 600},
        }
    },
    {
        name = '蜘蛛娃0',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 3600},
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
