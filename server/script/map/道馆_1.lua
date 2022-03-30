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

addGuard('大刀卫士', 371, 160, DIR_DOWNLEFT)
addGuard('大刀卫士', 375, 164, DIR_DOWNLEFT)
addGuard('大刀卫士', 368, 112, DIR_UPLEFT)
addGuard('大刀卫士', 372, 108, DIR_UPLEFT)
addGuard('大刀卫士', 414, 166, DIR_DOWNRIGHT)
addGuard('大刀卫士', 417, 163, DIR_DOWNRIGHT)
addGuard('大刀卫士', 411, 115, DIR_DOWNLEFT)
addGuard('大刀卫士', 414, 118, DIR_DOWNLEFT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 道馆_1
{
    {
        name = '半兽人',
        loc = {
            {x = 38, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 38, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 563, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 338, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 563, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '半兽人0',
        loc = {
            {x = 38, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 38, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 113, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 113, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 563, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '多钩猫',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 188, w = 38, h = 38, count = 15, time = 600},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 338, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 413, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '多钩猫0',
        loc = {
            {x = 113, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 113, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 188, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 38, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 113, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 188, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 38, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 188, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 400, y = 400, w = 400, h = 400, count = 200, time = 1200},
        }
    },
    {
        name = '栗子树',
        loc = {
            {x = 150, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 350, w = 50, h = 50, count = 3, time = 1200},
            {x = 150, y = 550, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 350, w = 50, h = 50, count = 3, time = 1200},
            {x = 250, y = 450, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 350, y = 450, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 50, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 150, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 250, w = 50, h = 50, count = 3, time = 1200},
            {x = 450, y = 550, w = 50, h = 50, count = 3, time = 1200},
            {x = 38, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 38, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 113, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 113, y = 458, w = 38, h = 38, count = 10, time = 300},
            {x = 113, y = 563, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 188, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 188, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 263, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 300},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 338, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 338, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 338, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 413, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 413, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 413, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 413, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 488, w = 38, h = 38, count = 10, time = 300},
            {x = 488, y = 563, w = 38, h = 38, count = 10, time = 300},
        }
    },
    {
        name = '森林雪人',
        loc = {
            {x = 38, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 38, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 563, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 338, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 563, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '森林雪人0',
        loc = {
            {x = 38, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 113, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 113, y = 563, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 488, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 563, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '毒蜘蛛',
        loc = {
            {x = 38, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 38, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 563, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 338, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 563, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '牛',
        loc = {
            {x = 338, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 113, w = 38, h = 38, count = 10, time = 600},
        }
    },
    {
        name = '狼',
        loc = {
            {x = 38, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 38, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 458, w = 38, h = 38, count = 15, time = 600},
            {x = 113, y = 563, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 413, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 488, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 338, w = 38, h = 38, count = 15, time = 600},
            {x = 338, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 338, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 488, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 563, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '猪',
        loc = {
            {x = 338, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 113, w = 38, h = 38, count = 10, time = 600},
        }
    },
    {
        name = '稻草人',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 188, w = 38, h = 38, count = 15, time = 600},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 338, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 413, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '稻草人0',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 113, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 113, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 188, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 188, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 38, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 413, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 413, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '蛤蟆',
        loc = {
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 188, w = 38, h = 38, count = 15, time = 600},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 600},
        }
    },
    {
        name = '蛤蟆0',
        loc = {
            {x = 263, y = 113, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 188, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 38, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 413, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 188, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 263, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '钉耙猫',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 188, w = 38, h = 38, count = 15, time = 600},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 338, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 413, w = 38, h = 38, count = 15, time = 600},
        }
    },
    {
        name = '钉耙猫0',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 38, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 113, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 263, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 338, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 263, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 338, w = 38, h = 38, count = 1, time = 3600},
            {x = 488, y = 413, w = 38, h = 38, count = 1, time = 3600},
        }
    },
    {
        name = '鸡',
        loc = {
            {x = 338, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 113, w = 38, h = 38, count = 10, time = 600},
        }
    },
    {
        name = '鹿',
        loc = {
            {x = 113, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 113, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 188, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 188, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 113, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 188, w = 38, h = 38, count = 15, time = 600},
            {x = 263, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 263, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 38, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 338, y = 338, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 263, w = 38, h = 38, count = 15, time = 600},
            {x = 413, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 413, y = 413, w = 38, h = 38, count = 15, time = 600},
            {x = 488, y = 188, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 263, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 338, w = 38, h = 38, count = 10, time = 600},
            {x = 488, y = 413, w = 38, h = 38, count = 15, time = 600},
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
