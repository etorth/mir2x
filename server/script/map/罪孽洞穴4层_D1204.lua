local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 罪孽洞穴4层_D1204
{
    {
        name = '怒龙神',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 9, time = 3600},
        }
    },
    {
        name = '赤黄猪王',
        loc = {
            {x = 150, y = 150, w = 150, h = 150, count = 9, time = 3600},
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
