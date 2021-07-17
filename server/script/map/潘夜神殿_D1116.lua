local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜神殿_D1116
{
    {
        name = '守卫右狮',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 20, time = 300, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 70, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜右护卫95',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 70, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 70, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜左护卫94',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 70, time = 3600, cratio = 0},
        }
    },
    {
        name = '潘夜牛魔王',
        loc = {
            {x = 100, y = 100, w = 90, h = 90, count = 1, time = 18000, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
