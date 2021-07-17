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

addGuard('禁军卫士', 285, 268, DIR_UPLEFT)
addGuard('禁军卫士', 275, 278, DIR_UPLEFT)
addGuard('禁军卫士', 353, 309, DIR_DOWNRIGHT)
addGuard('禁军卫士', 346, 316, DIR_DOWNRIGHT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 盟重县_74
{
    {
        name = '多角虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 300, y = 300, w = 300, h = 300, count = 150, time = 1200, cratio = 0},
        }
    },
    {
        name = '威思而小虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '巨型多角虫',
        loc = {
            {x = 300, y = 300, w = 290, h = 290, count = 2, time = 7200, cratio = 0},
        }
    },
    {
        name = '沙漠威思而小虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '猎鹰',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
        }
    },
    {
        name = '盔甲虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
            {x = 300, y = 300, w = 250, h = 250, count = 20, time = 600, cratio = 0},
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
