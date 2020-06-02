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

-- only initialize once
-- initialize all global/constant variables

function main()

    addLog(LOGTYPE_INFO, 'map script ' .. getMapName() .. ' starts, use default.lua')

    g_MaxMonsterCount = 1000
    g_LogicDelay      = 1000
    g_LastInvokeTime  = getTime()

    -- allowed monsters on current map

    g_MonsterList = getMonsterList()

    function getMonsterCountInList()
        local monCount = 0
        for i, v in pairs(g_MonsterList) do
            monCount = monCount + math.max(0, getMonsterCount(v))
        end
        return monCount
    end

    while not scriptDone() do
        if getTime() - g_LastInvokeTime > g_LogicDelay then

            -- mark current time
            -- then next time we start from here

            g_LastInvokeTime = getTime()

            if getMonsterCountInList() < g_MaxMonsterCount then

                local monsterName = g_MonsterList[math.random(#g_MonsterList)]
                for i = 1, 50 do
                    local x, y = getRandLoc()
                    addMonster(monsterName, x, y, true)
                end

                if math.random(1, 20) == 1 then
                    addMonster(monsterName, 400 + math.random(1, 5), 120 + math.random(1, 5), true)
                end

            end
        end
        coroutine.yield()
    end

    addLog(LOGTYPE_INFO, 'map script ' .. getMapName() .. ' stops, use default.lua')
end
