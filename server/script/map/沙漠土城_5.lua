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
