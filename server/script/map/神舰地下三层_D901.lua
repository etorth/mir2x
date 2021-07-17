local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 神舰地下三层_D901
{
    {
        name = '恶形鬼',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 17, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 18, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '恶形鬼8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '海神将领',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 10, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 10, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '海神将领8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '爆毒神魔',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 10, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 10, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '爆毒神魔8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '犬猴魔',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 18, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 17, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '犬猴魔8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '触角神魔',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 15, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 15, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '触角神魔8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '轻甲守卫',
        loc = {
            {x = 65, y = 40, w = 10, h = 10, count = 17, time = 600, cratio = 0},
            {x = 80, y = 90, w = 10, h = 10, count = 18, time = 600, cratio = 0},
            {x = 130, y = 125, w = 55, h = 55, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '轻甲守卫8',
        loc = {
            {x = 130, y = 125, w = 55, h = 55, count = 6, time = 600, cratio = 0},
        }
    },
    {
        name = '霸王守卫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
