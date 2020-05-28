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

    g_MaxMonsterCount = 1000
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

    addNPC(3, 400, 120, 0, false)
    addNPC(0, 400, 300, 0, false)
    addNPC(1, 401, 300, 0, false)
    addNPC(2, 402, 300, 0, false)
    addNPC(3, 403, 300, 0, false)
    addNPC(4, 404, 300, 0, false)
    addNPC(5, 405, 300, 0, false)
    addNPC(6, 406, 300, 0, false)
    addNPC(7, 407, 300, 0, false)
    addNPC(8, 408, 300, 0, false)
    addNPC(9, 409, 300, 0, false)
    addNPC(3, 397, 133, 0, false)
    addNPC(3, 388, 122, 0, false)

    -- add 六面神石

    addNPC(56, 416, 179, 0, false)
    g_Inited = true
end

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
