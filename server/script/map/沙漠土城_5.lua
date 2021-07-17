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

addGuard('沙漠战士', 240, 166, DIR_DOWNLEFT)
addGuard('沙漠战士', 237, 163, DIR_DOWNLEFT)
addGuard('沙漠战士', 208, 167, DIR_DOWNRIGHT)
addGuard('沙漠战士', 204, 171, DIR_DOWNRIGHT)
addGuard('沙漠战士', 238, 192, DIR_DOWNLEFT)
addGuard('沙漠战士', 242, 196, DIR_DOWNLEFT)
addGuard('沙漠战士', 219, 206, DIR_UPRIGHT)
addGuard('沙漠战士', 197, 184, DIR_UPRIGHT)
addGuard('沙漠战士', 182, 202, DIR_DOWNLEFT)
addGuard('沙漠战士', 205, 224, DIR_DOWNLEFT)
addGuard('沙漠战士', 143, 269, DIR_DOWNLEFT)
addGuard('沙漠战士', 138, 265, DIR_DOWNLEFT)
addGuard('沙漠战士', 122, 273, DIR_DOWNLEFT)
addGuard('沙漠战士', 129, 280, DIR_DOWNLEFT)
addGuard('沙漠战士', 132, 198, DIR_UPLEFT)
addGuard('沙漠战士', 138, 192, DIR_UPLEFT)
addGuard('沙漠战士', 199, 279, DIR_DOWNRIGHT)
addGuard('沙漠战士', 207, 271, DIR_DOWNRIGHT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沙漠土城_5
{
    {
        name = '羊',
        loc = {
            {x = 150, y = 50, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
            {x = 150, y = 150, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
            {x = 150, y = 250, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
            {x = 250, y = 50, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
            {x = 250, y = 150, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
            {x = 250, y = 250, w = 40, h = 40, count = 10, time = 1800, cratio = 0},
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
