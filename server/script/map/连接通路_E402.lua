local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 连接通路_E402
{
    {
        name = '僵尸1',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 5, time = 1380, cratio = 50},
        }
    },
    {
        name = '僧侣僵尸',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 3, time = 1380},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 3, time = 1380},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 3, time = 1380},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 3, time = 1380},
        }
    },
    {
        name = '山洞蝙蝠',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 5, time = 1380, cratio = 50},
        }
    },
    {
        name = '掷斧骷髅',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 5, time = 1380},
        }
    },
    {
        name = '洞蛆',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 5, time = 1380},
        }
    },
    {
        name = '骷髅',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380},
        }
    },
    {
        name = '骷髅战士',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380},
        }
    },
    {
        name = '骷髅战将',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 10, time = 1380},
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
