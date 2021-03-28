-- =====================================================================================
--
--       Filename: default.lua
--        Created: 06/01/2020 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler:
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

addLog(LOGTYPE_INFO, string.format('Map %s sources %s', getMapName(), getFileName()))

local logicDelay = 1000
local monsterList = {'虎卫', '红蛇', '虎蛇'}
local maxMonsterCount = math.floor(getCanThroughGridCount() / 64)

local function getAllMonsterCount()
    local monCount = 0
    for i, v in pairs(monsterList) do
        monCount = monCount + math.max(0, getMonsterCount(v))
    end
    return monCount
end

function main()
    while not scriptDone() do
        -- mark current time
        -- then next time we start from here

        local monsterCount = getAllMonsterCount()
        if monsterCount < maxMonsterCount then
            for i = 1, math.min(50, maxMonsterCount - monsterCount) do
                local x, y = getRandLoc()
                local monsterName = monsterList[math.random(#monsterList)]
                addMonster(monsterName, x, y, true)
            end
        end
        asyncWait(logicDelay)
    end
end
