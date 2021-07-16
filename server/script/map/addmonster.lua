local addmon = {}

-- input monGen sample:
-- {
--     name = '半兽人',
--     loc = {
--         {x = 413, y = 488, r = 38, count = 15, time = 10, cratio = 0},
--         {x = 488, y = 488, r = 38, count = 15, time = 10, cratio = 0},
--         {x = 488, y = 563, r = 38, count = 15, time = 10, cratio = 0},
--     }
-- },

function addmon.addMonster(monGen)
    if type(monGen) ~= 'table' then
        fatalPrintf('expect table, get %s', type(monGen))
    end

    local maxMonCount = 0
    for i, genLoc in pairs(monGen.loc) do
        maxMonCount = maxMonCount + genLoc.count
    end

    for i, genLoc in pairs(monGen.loc) do
        local currTime = getTime()
        if currTime - argDef(genLoc.lastGenTime, 0) > genLoc.time then
            genLoc.lastGenTime = currTime

            local monCount = getMonsterCount(monGen.name)
            if monCount < maxMonCount then
            end
        end
    end
end
return addmon
