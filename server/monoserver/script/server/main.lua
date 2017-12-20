-- =====================================================================================
--
--       Filename: main.lua
--        Created: 08/31/2015 08:52:57
--  Last Modified: 12/18/2017 20:32:25
--
--    Description: lua 5.3
--                 main entry of lua invoked by service core
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

    g_LogicDelay      = 1000
    g_LastInvokeTime  = getTime()

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
