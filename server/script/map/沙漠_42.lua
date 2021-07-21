local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沙漠_42
{
    {
        name = '夜行鬼09',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 100, time = 1200},
        }
    },
    {
        name = '大法老',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '沙漠树魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '沙漠石人',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '沙漠蜥蜴',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 80, time = 600},
        }
    },
    {
        name = '沙漠风魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '沙漠鱼魔',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '沙鬼',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '诺玛0',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '诺玛1',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '诺玛10',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛2',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 85, time = 600},
        }
    },
    {
        name = '诺玛20',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛3',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 80, time = 600},
        }
    },
    {
        name = '诺玛30',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛将士',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
        }
    },
    {
        name = '诺玛将士0',
        loc = {
            {x = 200, y = 200, w = 180, h = 180, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛教主2',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200},
        }
    },
    {
        name = '诺玛法老',
        loc = {
            {x = 200, y = 200, w = 200, h = 200, count = 70, time = 600},
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
