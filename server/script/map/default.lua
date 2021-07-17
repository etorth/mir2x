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

addLog(LOGTYPE_INFO, 'Map %s sources default script %s', getMapName(), getFileName())

local logicDelay = 1000
local monsterList = {'虎卫', '红蛇', '虎蛇'}
local maxMonsterCount = math.floor(getCanThroughGridCount() / 64)

function main()
    while true do
        local monsterCount = getMonsterCount(0)
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
