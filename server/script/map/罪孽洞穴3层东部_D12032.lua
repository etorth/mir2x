local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 罪孽洞穴3层东部_D12032
{
    {
        name = '怒龙神',
        loc = {
            {x = 100, y = 100, w = 150, h = 150, count = 7, time = 3600, cratio = 0},
        }
    },
    {
        name = '赤黄猪王',
        loc = {
            {x = 100, y = 100, w = 150, h = 150, count = 7, time = 3600, cratio = 0},
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
