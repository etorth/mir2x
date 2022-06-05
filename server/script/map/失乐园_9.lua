addGuard('昂克战士', 177, 519, DIR_UPLEFT)
addGuard('昂克战士', 174, 522, DIR_UPLEFT)
addGuard('昂克战士', 238, 524, DIR_UPRIGHT)
addGuard('昂克战士', 242, 527, DIR_UPRIGHT)
addGuard('昂克战士', 246, 606, DIR_DOWNRIGHT)
addGuard('昂克战士', 249, 602, DIR_DOWNRIGHT)
addGuard('昂克战士', 254, 553, DIR_DOWNLEFT)
addGuard('昂克战士', 258, 556, DIR_DOWNLEFT)
addGuard('昂克战士', 266, 562, DIR_DOWNLEFT)
addGuard('昂克战士', 250, 548, DIR_DOWNLEFT)
addGuard('昂克战士', 168, 589, DIR_UPLEFT)
addGuard('昂克战士', 163, 539, DIR_UP)
addGuard('昂克战士', 213, 513, DIR_UPRIGHT)
addGuard('昂克战士', 209, 643, DIR_DOWN)

local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 失乐园_9
{
    {
        name = '巨象兽',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 40, time = 600},
            {x = 70, y = 575, w = 60, h = 60, count = 3, time = 600},
            {x = 750, y = 750, w = 50, h = 50, count = 1, time = 600},
            {x = 550, y = 450, w = 50, h = 50, count = 1, time = 600},
            {x = 550, y = 350, w = 50, h = 50, count = 1, time = 600},
            {x = 700, y = 400, w = 100, h = 100, count = 8, time = 600},
            {x = 650, y = 150, w = 150, h = 150, count = 15, time = 600},
            {x = 220, y = 720, w = 70, h = 70, count = 2, time = 600},
            {x = 450, y = 650, w = 145, h = 145, count = 15, time = 600},
            {x = 700, y = 600, w = 100, h = 100, count = 8, time = 600},
        }
    },
    {
        name = '巨象兽0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600},
            {x = 550, y = 350, w = 50, h = 50, count = 1, time = 3600},
            {x = 700, y = 400, w = 100, h = 100, count = 1, time = 3600},
            {x = 650, y = 150, w = 150, h = 150, count = 1, time = 3600},
            {x = 220, y = 720, w = 70, h = 70, count = 1, time = 3600},
        }
    },
    {
        name = '巨象兽8',
        loc = {
            {x = 700, y = 600, w = 100, h = 100, count = 8, time = 600},
        }
    },
    {
        name = '猿猴战士',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 100, time = 600},
            {x = 70, y = 575, w = 60, h = 60, count = 6, time = 600},
            {x = 750, y = 750, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 450, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 350, w = 50, h = 50, count = 4, time = 600},
            {x = 700, y = 400, w = 100, h = 100, count = 20, time = 600},
            {x = 650, y = 150, w = 150, h = 150, count = 35, time = 600},
            {x = 220, y = 720, w = 70, h = 70, count = 5, time = 600},
            {x = 450, y = 650, w = 145, h = 145, count = 40, time = 600},
            {x = 700, y = 600, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '猿猴战士0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 2, time = 3600},
            {x = 750, y = 750, w = 50, h = 50, count = 1, time = 3600},
            {x = 550, y = 450, w = 50, h = 50, count = 1, time = 3600},
            {x = 550, y = 350, w = 50, h = 50, count = 1, time = 3600},
            {x = 700, y = 400, w = 100, h = 100, count = 1, time = 3600},
            {x = 650, y = 150, w = 150, h = 150, count = 1, time = 3600},
            {x = 450, y = 650, w = 145, h = 145, count = 1, time = 3600},
            {x = 700, y = 600, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '猿猴战士8',
        loc = {
            {x = 700, y = 600, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '猿猴战将',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 100, time = 600},
            {x = 70, y = 575, w = 60, h = 60, count = 6, time = 600},
            {x = 750, y = 750, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 450, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 350, w = 50, h = 50, count = 4, time = 600},
            {x = 700, y = 400, w = 100, h = 100, count = 20, time = 600},
            {x = 650, y = 150, w = 150, h = 150, count = 40, time = 600},
            {x = 220, y = 720, w = 70, h = 70, count = 5, time = 600},
            {x = 450, y = 650, w = 145, h = 145, count = 35, time = 600},
            {x = 700, y = 600, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '猿猴战将0',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 2, time = 3600},
            {x = 70, y = 575, w = 60, h = 60, count = 1, time = 3600},
            {x = 550, y = 450, w = 50, h = 50, count = 1, time = 3600},
            {x = 700, y = 400, w = 100, h = 100, count = 1, time = 3600},
            {x = 650, y = 150, w = 150, h = 150, count = 1, time = 3600},
            {x = 450, y = 650, w = 145, h = 145, count = 1, time = 3600},
        }
    },
    {
        name = '猿猴战将8',
        loc = {
            {x = 700, y = 600, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '疯狂魔神盗',
        loc = {
            {x = 400, y = 400, w = 390, h = 390, count = 4, time = 7200},
        }
    },
    {
        name = '魔神怪1',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 70, time = 600},
            {x = 70, y = 575, w = 60, h = 60, count = 4, time = 600},
            {x = 750, y = 750, w = 50, h = 50, count = 2, time = 600},
            {x = 550, y = 450, w = 50, h = 50, count = 2, time = 600},
            {x = 550, y = 350, w = 50, h = 50, count = 2, time = 600},
            {x = 700, y = 400, w = 100, h = 100, count = 12, time = 600},
            {x = 650, y = 150, w = 150, h = 150, count = 20, time = 600},
            {x = 220, y = 720, w = 70, h = 70, count = 3, time = 600},
            {x = 450, y = 650, w = 145, h = 145, count = 20, time = 600},
            {x = 700, y = 600, w = 100, h = 100, count = 12, time = 600},
        }
    },
    {
        name = '魔神怪10',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 1, time = 3600},
            {x = 70, y = 575, w = 60, h = 60, count = 1, time = 3600},
            {x = 450, y = 650, w = 145, h = 145, count = 1, time = 3600},
            {x = 700, y = 600, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '魔神怪2',
        loc = {
            {x = 250, y = 250, w = 240, h = 240, count = 100, time = 600},
            {x = 70, y = 575, w = 60, h = 60, count = 6, time = 600},
            {x = 750, y = 750, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 450, w = 50, h = 50, count = 4, time = 600},
            {x = 550, y = 350, w = 50, h = 50, count = 4, time = 600},
            {x = 700, y = 400, w = 100, h = 100, count = 20, time = 600},
            {x = 650, y = 150, w = 150, h = 150, count = 40, time = 600},
            {x = 220, y = 720, w = 70, h = 70, count = 5, time = 600},
            {x = 450, y = 650, w = 145, h = 145, count = 40, time = 600},
            {x = 700, y = 600, w = 100, h = 100, count = 20, time = 600},
        }
    },
    {
        name = '魔神怪20',
        loc = {
            {x = 250, y = 250, w = 250, h = 250, count = 2, time = 3600},
            {x = 750, y = 750, w = 50, h = 50, count = 1, time = 3600},
            {x = 220, y = 720, w = 70, h = 70, count = 1, time = 3600},
            {x = 700, y = 600, w = 100, h = 100, count = 1, time = 3600},
        }
    },
    {
        name = '魔神怪8',
        loc = {
            {x = 700, y = 600, w = 100, h = 100, count = 12, time = 600},
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
