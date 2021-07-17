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

addGuard('禁军卫士', 457, 386, DIR_DOWNLEFT)
addGuard('禁军卫士', 454, 383, DIR_DOWNLEFT)
addGuard('禁军卫士', 471, 371, DIR_DOWNLEFT)
addGuard('禁军卫士', 467, 367, DIR_DOWNLEFT)
addGuard('禁军卫士', 495, 387, DIR_DOWNLEFT)
addGuard('禁军卫士', 499, 391, DIR_DOWNLEFT)
addGuard('禁军卫士', 511, 369, DIR_UPRIGHT)
addGuard('禁军卫士', 515, 373, DIR_UPRIGHT)
addGuard('禁军卫士', 497, 344, DIR_UPRIGHT)
addGuard('禁军卫士', 492, 339, DIR_UPRIGHT)
addGuard('禁军卫士', 511, 325, DIR_UPRIGHT)
addGuard('禁军卫士', 515, 329, DIR_UPRIGHT)
addGuard('禁军卫士', 466, 325, DIR_UPRIGHT)
addGuard('禁军卫士', 461, 320, DIR_UPRIGHT)
addGuard('禁军卫士', 451, 342, DIR_DOWNLEFT)
addGuard('禁军卫士', 448, 339, DIR_DOWNLEFT)
addGuard('禁军卫士', 406, 343, DIR_DOWNLEFT)
addGuard('禁军卫士', 389, 328, DIR_UPLEFT)
addGuard('禁军卫士', 387, 330, DIR_UPLEFT)
addGuard('禁军卫士', 412, 426, DIR_UPLEFT)
addGuard('禁军卫士', 403, 443, DIR_DOWNLEFT)
addGuard('禁军卫士', 396, 436, DIR_DOWNLEFT)
addGuard('禁军卫士', 393, 449, DIR_DOWNLEFT)
addGuard('禁军卫士', 391, 447, DIR_DOWNLEFT)
addGuard('禁军卫士', 507, 448, DIR_DOWNRIGHT)
addGuard('禁军卫士', 503, 452, DIR_DOWNRIGHT)
addGuard('禁军卫士', 483, 410, DIR_DOWNLEFT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 比奇县_0
{
    {
        name = '半兽人',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '半兽人0',
        loc = {
            {x = 150, y = 650, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '半兽勇士',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
            {x = 600, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
            {x = 200, y = 600, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
            {x = 600, y = 600, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '半兽战士',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '半兽战士0',
        loc = {
            {x = 50, y = 250, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '圣诞树',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
        }
    },
    {
        name = '圣诞树1',
        loc = {
            {x = 450, y = 393, w = 1, h = 1, count = 1, time = 3600, cratio = 0},
            {x = 465, y = 372, w = 1, h = 1, count = 1, time = 3600, cratio = 0},
            {x = 434, y = 375, w = 1, h = 1, count = 1, time = 3600, cratio = 0},
            {x = 471, y = 406, w = 1, h = 1, count = 1, time = 3600, cratio = 0},
            {x = 422, y = 402, w = 1, h = 1, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '多钩猫',
        loc = {
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '多钩猫0',
        loc = {
            {x = 150, y = 250, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 400, y = 400, w = 400, h = 400, count = 200, time = 1200, cratio = 0},
        }
    },
    {
        name = '栗子树',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 3, time = 1200, cratio = 0},
            {x = 50, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 13, time = 300, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 13, time = 300, cratio = 0},
        }
    },
    {
        name = '森林雪人',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '森林雪人0',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '毒蜘蛛',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '牛',
        loc = {
            {x = 350, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '狼',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '猪',
        loc = {
            {x = 350, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '稻草人',
        loc = {
            {x = 250, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '稻草人0',
        loc = {
            {x = 350, y = 150, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '蛤蟆',
        loc = {
            {x = 250, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '蛤蟆0',
        loc = {
            {x = 250, y = 350, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '钉耙猫',
        loc = {
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '钉耙猫0',
        loc = {
            {x = 250, y = 50, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 1, time = 3600, cratio = 0},
        }
    },
    {
        name = '食人花',
        loc = {
            {x = 50, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 50, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 150, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 650, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 50, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 50, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 650, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 750, y = 750, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '鸡',
        loc = {
            {x = 350, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 350, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
    {
        name = '鹿',
        loc = {
            {x = 250, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 250, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 350, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 250, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 150, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 450, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 350, y = 550, w = 50, h = 50, count = 5, time = 600, cratio = 0},
            {x = 450, y = 150, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 450, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 550, y = 550, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 250, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 350, w = 50, h = 50, count = 10, time = 600, cratio = 0},
            {x = 650, y = 450, w = 50, h = 50, count = 10, time = 600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
