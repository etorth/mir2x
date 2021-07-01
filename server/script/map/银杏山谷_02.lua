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

addGuard('大刀卫士', 232, 180, DIR_UPLEFT)
addGuard('大刀卫士', 228, 184, DIR_UPLEFT)
addGuard('大刀卫士', 244, 168, DIR_UPLEFT)
addGuard('大刀卫士', 247, 165, DIR_UPLEFT)
addGuard('大刀卫士', 274, 167, DIR_UPRIGHT)
addGuard('大刀卫士', 277, 170, DIR_UPRIGHT)
addGuard('大刀卫士', 283, 176, DIR_UPRIGHT)
addGuard('大刀卫士', 287, 180, DIR_UPRIGHT)
addGuard('大刀卫士', 289, 205, DIR_DOWNRIGHT)
addGuard('大刀卫士', 286, 208, DIR_DOWNRIGHT)
addGuard('大刀卫士', 277, 233, DIR_DOWNRIGHT)
addGuard('大刀卫士', 274, 236, DIR_DOWNRIGHT)
addGuard('大刀卫士', 229, 239, DIR_DOWNLEFT)
addGuard('大刀卫士', 227, 237, DIR_DOWNLEFT)

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
