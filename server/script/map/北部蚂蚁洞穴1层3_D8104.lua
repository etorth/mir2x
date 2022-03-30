local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部蚂蚁洞穴1层3_D8104
{
    {
        name = '劳动蚂蚁',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600},
        }
    },
    {
        name = '劳动蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600},
        }
    },
    {
        name = '爆毒蚂蚁',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600},
        }
    },
    {
        name = '爆毒蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600},
        }
    },
    {
        name = '盔甲蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600},
            {x = 100, y = 100, w = 50, h = 50, count = 1, time = 3600},
        }
    },
    {
        name = '蚂蚁将军',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200},
        }
    },
    {
        name = '蚂蚁战士',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600},
        }
    },
    {
        name = '蚂蚁战士0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600},
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
