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
