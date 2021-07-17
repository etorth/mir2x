local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沙漠_71
{
    {
        name = '多角虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 200, y = 100, w = 100, h = 100, count = 75, time = 1200, cratio = 0},
        }
    },
    {
        name = '猎鹰0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '盔甲虫0',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
