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

if g_Inited == nil then

    -- only initialize once
    -- initialize all global/constant variables

    g_MaxMonsterCount = 10
    g_LogicDelay      = 1000
    g_LastInvokeTime  = getTime()

    -- allowed monsters on current map

    g_MonsterList =
    {
        "虎卫",
        "沙漠石人"
    }

    function getMonsterCountInList()
        local nMonsterCount = 0
        for i, v in pairs(g_MonsterList) do
            nMonsterCount = nMonsterCount + math.max(0, getMonsterCount(v)) 
        end
        return nMonsterCount
    end

    g_Inited = true
end

if getTime() - g_LastInvokeTime > g_LogicDelay then

    -- mark current time
    -- then next time we start from here

    g_LastInvokeTime = getTime()

    if getMonsterCountInList() < g_MaxMonsterCount then

        addMonster(g_MonsterList[math.random(#g_MonsterList)], 400 + math.random(1, 5), 120 + math.random(1, 5), true)

    end

end
