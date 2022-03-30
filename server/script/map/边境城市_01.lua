-- =====================================================================================
--
--       Filename: 道馆.lua
--        Created: 08/31/2015 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: gcc
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

addGuard('大刀卫士', 426, 283, DIR_DOWNLEFT)
addGuard('大刀卫士', 413, 272, DIR_DOWNRIGHT)
addGuard('大刀卫士', 431, 240, DIR_UPLEFT)
addGuard('大刀卫士', 427, 244, DIR_UPLEFT)
addGuard('大刀卫士', 462, 241, DIR_UPRIGHT)
addGuard('大刀卫士', 455, 232, DIR_UPRIGHT)
addGuard('大刀卫士', 441, 325, DIR_DOWN)
addGuard('大刀卫士', 451, 316, DIR_DOWNLEFT)
addGuard('大刀卫士', 427, 320, DIR_DOWNRIGHT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 边境城市_01
{
    {
        name = '半兽人',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 90, time = 600},
        }
    },
    {
        name = '半兽人0',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '多钩猫',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 300, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 500, w = 100, h = 100, count = 70, time = 600},
            {x = 500, y = 100, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '多钩猫0',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 2, time = 3600},
            {x = 300, y = 300, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 500, w = 100, h = 100, count = 2, time = 3600},
            {x = 500, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 300, y = 300, w = 300, h = 300, count = 150, time = 1200},
        }
    },
    {
        name = '栗子树',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 50, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 350, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 350, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 450, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 550, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 450, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 550, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 550, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 100, y = 300, w = 100, h = 100, count = 50, time = 300},
            {x = 300, y = 100, w = 100, h = 100, count = 50, time = 300},
            {x = 300, y = 300, w = 100, h = 100, count = 50, time = 300},
            {x = 300, y = 500, w = 100, h = 100, count = 50, time = 300},
            {x = 500, y = 100, w = 100, h = 100, count = 50, time = 300},
        }
    },
    {
        name = '森林雪人',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 80, time = 600},
        }
    },
    {
        name = '森林雪人0',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '毒蜘蛛',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 80, time = 600},
        }
    },
    {
        name = '牛',
        loc = {
            {x = 450, y = 250, w = 50, h = 50, count = 30, time = 600},
            {x = 450, y = 350, w = 50, h = 50, count = 30, time = 600},
        }
    },
    {
        name = '狼',
        loc = {
            {x = 100, y = 300, w = 100, h = 100, count = 80, time = 600},
            {x = 300, y = 100, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 500, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '猪',
        loc = {
            {x = 450, y = 250, w = 50, h = 50, count = 30, time = 600},
            {x = 450, y = 350, w = 50, h = 50, count = 30, time = 600},
        }
    },
    {
        name = '稻草人',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 300, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 500, w = 100, h = 100, count = 70, time = 600},
            {x = 500, y = 100, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '稻草人0',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 300, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 500, w = 100, h = 100, count = 1, time = 3600},
            {x = 500, y = 100, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '蛤蟆',
        loc = {
            {x = 300, y = 300, w = 100, h = 100, count = 70, time = 600},
            {x = 500, y = 100, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '蛤蟆0',
        loc = {
            {x = 300, y = 300, w = 100, h = 100, count = 2, time = 3600},
            {x = 500, y = 100, w = 100, h = 100, count = 2, time = 3600},
        }
    },
    {
        name = '钉耙猫',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 300, w = 100, h = 100, count = 70, time = 600},
            {x = 300, y = 500, w = 100, h = 100, count = 70, time = 600},
            {x = 500, y = 100, w = 100, h = 100, count = 70, time = 600},
        }
    },
    {
        name = '钉耙猫0',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 1, time = 3600},
            {x = 300, y = 300, w = 100, h = 100, count = 2, time = 3600},
            {x = 300, y = 500, w = 100, h = 100, count = 1, time = 3600},
            {x = 500, y = 100, w = 100, h = 100, count = 2, time = 3600},
        }
    },
    {
        name = '鸡',
        loc = {
            {x = 450, y = 250, w = 50, h = 50, count = 30, time = 600},
            {x = 450, y = 350, w = 50, h = 50, count = 30, time = 600},
        }
    },
    {
        name = '鹿',
        loc = {
            {x = 300, y = 100, w = 100, h = 100, count = 60, time = 600},
            {x = 300, y = 300, w = 100, h = 100, count = 60, time = 600},
            {x = 300, y = 500, w = 100, h = 100, count = 60, time = 600},
            {x = 500, y = 100, w = 100, h = 100, count = 60, time = 600},
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
