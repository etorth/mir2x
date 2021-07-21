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

addGuard('大刀卫士', 269, 292, DIR_DOWNRIGHT)
addGuard('大刀卫士', 260, 302, DIR_DOWNRIGHT)
addGuard('大刀卫士', 234, 293, DIR_UPRIGHT)
addGuard('大刀卫士', 230, 288, DIR_UPRIGHT)
addGuard('大刀卫士', 212, 254, DIR_UPLEFT)
addGuard('大刀卫士', 232, 231, DIR_DOWNLEFT)
addGuard('大刀卫士', 218, 206, DIR_DOWNLEFT)
addGuard('大刀卫士', 223, 200, DIR_UPLEFT)
addGuard('大刀卫士', 244, 196, DIR_UPRIGHT)
addGuard('大刀卫士', 262, 204, DIR_DOWNRIGHT)
addGuard('大刀卫士', 279, 206, DIR_UPRIGHT)
addGuard('大刀卫士', 281, 225, DIR_DOWNRIGHT)
addGuard('大刀卫士', 271, 263, DIR_DOWNRIGHT)
addGuard('大刀卫士', 263, 269, DIR_UPRIGHT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 潘夜岛_8
{
    {
        name = '夜行鬼09',
        loc = {
            {x = 400, y = 400, w = 200, h = 200, count = 200, time = 1200},
        }
    },
    {
        name = '浪子人鬼',
        loc = {
            {x = 700, y = 300, w = 90, h = 90, count = 310, time = 600},
            {x = 500, y = 500, w = 90, h = 90, count = 315, time = 600},
            {x = 300, y = 700, w = 90, h = 90, count = 315, time = 600},
            {x = 500, y = 700, w = 90, h = 90, count = 315, time = 600},
        }
    },
    {
        name = '浪子人鬼0',
        loc = {
            {x = 700, y = 300, w = 90, h = 90, count = 2, time = 3600},
            {x = 500, y = 500, w = 90, h = 90, count = 2, time = 3600},
            {x = 300, y = 700, w = 90, h = 90, count = 2, time = 3600},
            {x = 500, y = 700, w = 90, h = 90, count = 2, time = 3600},
        }
    },
    {
        name = '腐蚀人鬼',
        loc = {
            {x = 100, y = 300, w = 90, h = 90, count = 310, time = 600},
            {x = 100, y = 500, w = 90, h = 90, count = 310, time = 600},
            {x = 300, y = 500, w = 90, h = 90, count = 315, time = 600},
            {x = 700, y = 500, w = 90, h = 90, count = 315, time = 600},
        }
    },
    {
        name = '腐蚀人鬼0',
        loc = {
            {x = 100, y = 300, w = 90, h = 90, count = 2, time = 3600},
            {x = 100, y = 500, w = 90, h = 90, count = 2, time = 3600},
            {x = 300, y = 500, w = 90, h = 90, count = 2, time = 3600},
            {x = 700, y = 500, w = 90, h = 90, count = 2, time = 3600},
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
