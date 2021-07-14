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

addLog(LOGTYPE_INFO, string.format('Map %s sources %s', getMapName(), getFileName()))

local logicDelay = 1000
local monsterList = {'多角虫', '巨型多角虫', '猎鹰', '盔甲虫', '沙漠威斯尔小虫', '威思尔小虫', '羊'}
local maxMonsterCount = math.floor(getCanThroughGridCount() / 64)

addGuard('禁军卫士', 285, 268, DIR_UPLEFT)
addGuard('禁军卫士', 275, 278, DIR_UPLEFT)
addGuard('禁军卫士', 353, 309, DIR_DOWNRIGHT)
addGuard('禁军卫士', 346, 316, DIR_DOWNRIGHT)

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
