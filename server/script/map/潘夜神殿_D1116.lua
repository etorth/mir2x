local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿_D1116
{
    {
        name = '守卫右狮',
        loc = {
            {x = 0, y = 0, w = 90, h = 90, count = 20, time = 300},
        }
    },
    {
        name = '潘夜右护卫',
        loc = {
            {x = 0, y = 0, w = 90, h = 90, count = 70, time = 3600},
        }
    },
    {
        name = '潘夜右护卫95',
        loc = {
            {x = 0, y = 0, w = 90, h = 90, count = 70, time = 3600},
        }
    },
    {
        name = '潘夜左护卫',
        loc = {
            {x = 0, y = 0, w = 90, h = 90, count = 70, time = 3600},
        }
    },
    {
        name = '潘夜左护卫94',
        loc = {
            {x = 0, y = 0, w = 90, h = 90, count = 70, time = 3600},
        }
    },
    {
        name = '潘夜牛魔王',
        loc = {
            {x = 51, y = 51, w = 1, h = 1, count = 1, time = 18000},
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
