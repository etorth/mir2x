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

addGuard('沙漠战士', 451, 54, DIR_DOWNLEFT)
addGuard('沙漠战士', 457, 60, DIR_DOWNLEFT)
addGuard('沙漠战士', 462, 48, DIR_UPRIGHT)
addGuard('沙漠战士', 456, 46, DIR_UPRIGHT)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 绿洲_4
{
    {
        name = '夜行鬼09',
        loc = {
            {x = 400, y = 400, w = 400, h = 400, count = 200, time = 1200},
        }
    },
    {
        name = '大法老',
        loc = {
            {x = 400, y = 400, w = 390, h = 390, count = 2, time = 7200},
        }
    },
    {
        name = '沙漠树魔',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 5, time = 600},
        }
    },
    {
        name = '沙漠石人',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 5, time = 600},
        }
    },
    {
        name = '沙漠石人0',
        loc = {
            {x = 66, y = 332, w = 66, h = 66, count = 1, time = 3600},
            {x = 199, y = 598, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 332, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 598, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 731, w = 66, h = 66, count = 1, time = 3600},
            {x = 465, y = 731, w = 66, h = 66, count = 1, time = 3600},
        }
    },
    {
        name = '沙漠树魔0',
        loc = {
            {x = 66, y = 465, w = 66, h = 66, count = 1, time = 3600},
            {x = 66, y = 598, w = 66, h = 66, count = 1, time = 3600},
            {x = 199, y = 465, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 465, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 598, w = 66, h = 66, count = 1, time = 3600},
        }
    },
    {
        name = '羊',
        loc = {
            {x = 332, y = 66, w = 66, h = 66, count = 8, time = 600},
            {x = 332, y = 199, w = 66, h = 66, count = 8, time = 600},
            {x = 598, y = 66, w = 66, h = 66, count = 8, time = 600},
            {x = 598, y = 199, w = 66, h = 66, count = 8, time = 600},
            {x = 731, y = 66, w = 66, h = 66, count = 8, time = 600},
            {x = 465, y = 66, w = 66, h = 66, count = 8, time = 600},
            {x = 465, y = 199, w = 66, h = 66, count = 8, time = 600},
        }
    },
    {
        name = '诺玛0',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 20, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 20, time = 600},
        }
    },
    {
        name = '诺玛00',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 1, time = 3600},
            {x = 66, y = 332, w = 66, h = 66, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛1',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 20, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 20, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 20, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 20, time = 600},
        }
    },
    {
        name = '诺玛10',
        loc = {
            {x = 199, y = 332, w = 66, h = 66, count = 1, time = 3600},
            {x = 465, y = 332, w = 66, h = 66, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛将士',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 5, time = 600},
        }
    },
    {
        name = '诺玛将士0',
        loc = {
            {x = 199, y = 332, w = 66, h = 66, count = 1, time = 3600},
            {x = 199, y = 598, w = 66, h = 66, count = 1, time = 3600},
            {x = 199, y = 731, w = 66, h = 66, count = 1, time = 3600},
        }
    },
    {
        name = '诺玛法老',
        loc = {
            {x = 66, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 66, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 199, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 199, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 465, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 598, w = 66, h = 66, count = 5, time = 600},
            {x = 332, y = 731, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 332, w = 66, h = 66, count = 5, time = 600},
            {x = 465, y = 731, w = 66, h = 66, count = 5, time = 600},
        }
    },
    {
        name = '诺玛法老0',
        loc = {
            {x = 66, y = 598, w = 66, h = 66, count = 1, time = 3600},
            {x = 199, y = 199, w = 66, h = 66, count = 1, time = 3600},
            {x = 332, y = 332, w = 66, h = 66, count = 1, time = 3600},
            {x = 465, y = 332, w = 66, h = 66, count = 1, time = 3600},
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
