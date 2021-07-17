local addmon = {}

-- local monsterGenList = -- 道馆_1
-- {
--     {
--         name = '半兽人',
--         loc = {
--             {x = 488, y = 488, w = 38, h = 38, count = 15, time = 10, cratio = 0},
--             {x = 488, y = 563, w = 38, h = 38, count = 15, time = 10, cratio = 0},
--         }
--     },
--     {
--         name = '多钩猫',
--         loc = {
--             {x = 113, y = 263, w = 38, h = 38, count = 10, time = 10, cratio = 0},
--             {x = 113, y = 338, w = 38, h = 38, count = 10, time = 10, cratio = 0},
--             {x = 188, y = 113, w = 38, h = 38, count = 10, time = 10, cratio = 0},
--         }
--     },
-- }

function addmon.monGener(monGenList)
    assertType(monGenList, 'table')
    for i, genList in ipairs(monGenList) do
        genList.uidList = {}
        genList.lastUpdateTime = 0
    end

    return coroutine.create(function ()
        while true do
            coroutine.yield()
            local currTime = getTime()
            for i, genList in ipairs(monGenList) do
                if currTime > genList.lastUpdateTime + genList.time then
                    -- seems lua remove a key is pretty complicated
                    -- see: https://stackoverflow.com/questions/12394841/safely-remove-items-from-an-array-table-while-iterating
                    local keyList = {}
                    for j, uidString in pairs(genList.uidList) do
                        if not isUIDAlive(uidString) then
                            table.insert(keyList, j)
                        end
                    end

                    for _, ikey in pairs(keyList) do
                        genList.uidList[ikey] = nil
                    end

                    local aliveMonCount = 0
                    for _, _ in pairs(genList.uidList) do
                        aliveMonCount = aliveMonCount + 1
                    end

                    local needCount = genList.count - aliveMonCount
                    while needCount > 0 do
                        local addX, addY = randGLoc(genList.x, genList.y, genList.w, genList.h)
                        local uidString = addMonster(genList.name, addX, addY, true)

                        if uidString ~= nil then
                            table.insert(genList.uidList, uidString)
                            needCount = needCount - 1
                        end
                    end
                end
            end
        end
    end)
end
return addmon
