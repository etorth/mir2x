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

addGuard('禁军卫士', 457, 386, DIR_DOWNLEFT)
addGuard('禁军卫士', 454, 383, DIR_DOWNLEFT)
addGuard('禁军卫士', 471, 371, DIR_DOWNLEFT)
addGuard('禁军卫士', 467, 367, DIR_DOWNLEFT)
addGuard('禁军卫士', 495, 387, DIR_DOWNLEFT)
addGuard('禁军卫士', 499, 391, DIR_DOWNLEFT)
addGuard('禁军卫士', 511, 369, DIR_UPRIGHT)
addGuard('禁军卫士', 515, 373, DIR_UPRIGHT)
addGuard('禁军卫士', 497, 344, DIR_UPRIGHT)
addGuard('禁军卫士', 492, 339, DIR_UPRIGHT)
addGuard('禁军卫士', 511, 325, DIR_UPRIGHT)
addGuard('禁军卫士', 515, 329, DIR_UPRIGHT)
addGuard('禁军卫士', 466, 325, DIR_UPRIGHT)
addGuard('禁军卫士', 461, 320, DIR_UPRIGHT)
addGuard('禁军卫士', 451, 342, DIR_DOWNLEFT)
addGuard('禁军卫士', 448, 339, DIR_DOWNLEFT)
addGuard('禁军卫士', 406, 343, DIR_DOWNLEFT)
addGuard('禁军卫士', 389, 328, DIR_UPLEFT)
addGuard('禁军卫士', 387, 330, DIR_UPLEFT)
addGuard('禁军卫士', 412, 426, DIR_UPLEFT)
addGuard('禁军卫士', 403, 443, DIR_DOWNLEFT)
addGuard('禁军卫士', 396, 436, DIR_DOWNLEFT)
addGuard('禁军卫士', 393, 449, DIR_DOWNLEFT)
addGuard('禁军卫士', 391, 447, DIR_DOWNLEFT)
addGuard('禁军卫士', 507, 448, DIR_DOWNRIGHT)
addGuard('禁军卫士', 503, 452, DIR_DOWNRIGHT)
addGuard('禁军卫士', 483, 410, DIR_DOWNLEFT)

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
