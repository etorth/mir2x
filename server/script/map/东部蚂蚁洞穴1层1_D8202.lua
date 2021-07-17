local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 东部蚂蚁洞穴1层1_D8202
{
    {
        name = '劳动蚂蚁',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '劳动蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600, cratio = 0},
        }
    },
    {
        name = '爆毒蚂蚁',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '爆毒蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600, cratio = 0},
        }
    },
    {
        name = '盔甲蚂蚁0',
        loc = {
            {x = 100, y = 100, w = 50, h = 50, count = 2, time = 3600, cratio = 0},
            {x = 100, y = 100, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '蚂蚁将军',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '蚂蚁战士',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '蚂蚁战士0',
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
