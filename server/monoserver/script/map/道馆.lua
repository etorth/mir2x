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

-- only initialize once
-- initialize all global/constant variables

function main()
    addLog(LOGTYPE_INFO, string.format('Map %s sources %s', getMapName(), getFileName()))

    g_MaxMonsterCount = math.floor(getCanThroughGridCount() / 64)
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

    -- npc loading is done in ServerMap::onActivate()
    -- which loads all scripts matches map name
    while not scriptDone() do
        if getTime() - g_LastInvokeTime > g_LogicDelay then

            -- mark current time
            -- then next time we start from here

            g_LastInvokeTime = getTime()
            local monsterCount = getMonsterCountInList()

            if monsterCount < g_MaxMonsterCount then
                local monsterName = g_MonsterList[math.random(#g_MonsterList)]
                for i = 1, math.min(50, g_MaxMonsterCount - monsterCount) do
                    local x, y = getRandLoc()
                    addMonster(monsterName, x, y, true)
                end
            end

            if math.random(1, 10) == 1 then
                addMonster('沙漠石人', 400 + math.random(1, 5), 120 + math.random(1, 5), true)
            end
        end
        coroutine.yield()
    end

    addLog(LOGTYPE_INFO, 'map script %s stopped', getMapName())
end
