local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 神舰地下二层_D9021
{
    {
        name = '恶形鬼',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '恶形鬼8',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 5, time = 600, cratio = 0},
        }
    },
    {
        name = '爆毒神魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 30, time = 600, cratio = 0},
        }
    },
    {
        name = '爆毒神魔8',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 5, time = 600, cratio = 0},
        }
    },
    {
        name = '犬猴魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 50, time = 600, cratio = 0},
        }
    },
    {
        name = '犬猴魔8',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 5, time = 600, cratio = 0},
        }
    },
    {
        name = '触角神魔',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 55, time = 600, cratio = 0},
        }
    },
    {
        name = '触角神魔8',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 5, time = 600, cratio = 0},
        }
    },
    {
        name = '轻甲守卫',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 60, time = 600, cratio = 0},
        }
    },
    {
        name = '轻甲守卫8',
        loc = {
            {x = 100, y = 100, w = 100, h = 100, count = 5, time = 600, cratio = 0},
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
