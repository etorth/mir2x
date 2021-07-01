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
local monsterList = {'鸡', '猪', '牛' , '鹿', '稻草人', '钉耙猫', '狼', '食人花', '多钩猫', '毒蜘蛛'}
local maxMonsterCount = math.floor(getCanThroughGridCount() / 64)

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
