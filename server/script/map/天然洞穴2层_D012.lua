local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 天然洞穴2层_D012
{
    {
        name = '掷斧骷髅',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 120, time = 600},
        }
    },
    {
        name = '掷斧骷髅0',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 3600},
        }
    },
    {
        name = '洞蛆',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 120, time = 600},
        }
    },
    {
        name = '蝎子',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 120, time = 600},
        }
    },
    {
        name = '骷髅战士',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 120, time = 600},
        }
    },
    {
        name = '骷髅战士0',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 3600},
        }
    },
    {
        name = '骷髅战将',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 120, time = 600},
        }
    },
    {
        name = '骷髅战将0',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 2, time = 3600},
        }
    },
    {
        name = '骷髅精灵',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
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
