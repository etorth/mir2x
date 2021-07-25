local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 北部地下1层第一间房_D441
{
    {
        name = '僵尸1',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600},
        }
    },
    {
        name = '僧侣僵尸',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600},
        }
    },
    {
        name = '僵尸3',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600},
        }
    },
    {
        name = '僵尸4',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600},
        }
    },
    {
        name = '僵尸5',
        loc = {
            {x = 50, y = 50, w = 50, h = 50, count = 20, time = 600},
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
