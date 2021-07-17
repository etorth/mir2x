local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 虫峡谷2层_D6022
{
    {
        name = '蜈蚣',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '蜈蚣0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '蝴蝶虫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '蝴蝶虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '跳跳蜂',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '跳跳蜂0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '邪恶钳虫',
        loc = {
            {x = 50, y = 50, w = 150, h = 150, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '钳虫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '钳虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '黑色恶蛆',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600, cratio = 0},
        }
    },
    {
        name = '黑色恶蛆0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
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
