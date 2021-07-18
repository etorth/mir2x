local addmon = {}

-- input monster gen table sample format:
-- local monGenList = -- 道馆_1
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

    local badNameKeyList = {}
    for i, genList in pairs(monGenList) do
        if getMonsterID(genList.name) == 0 then
            addLog(LOGTYPE_WARNING, 'invalid monster name: %s', genList.name)
            table.insert(badNameKeyList, i)
        else
            local badRectKeyList = {}
            for j, locInfo in pairs(genList.loc) do
                locInfo.uidList = {}
                locInfo.lastUpdateTime = -1000 * locInfo.time -- hack for init
                locInfo.gridCount = countGLoc(locInfo.x, locInfo.y, locInfo.w, locInfo.h)

                if locInfo.gridCount < 0 or locInfo.gridCount > locInfo.w * locInfo.h then
                    fatalPrintf('invalid grid count: count = %d, x = %d, y = %d, w = %d, h = %d', locInfo.gridCount, locInfo.x, locInfo.y, locInfo.w, locInfo.h)
                end

                if locInfo.gridCount == 0 then
                    addLog(LOGTYPE_WARNING, 'region has no valid grid: map = %s, x = %d, y = %d, w = %d, h = %d', getMapName(), locInfo.x, locInfo.y, locInfo.w, locInfo.h)
                    table.insert(badRectKeyList, j)
                end
            end

            for _, badRectKey in pairs(badRectKeyList) do
                genList.loc[badRectKey] = nil
            end

            local locCount = 0
            for _, _ in pairs(genList.loc) do
                locCount = locCount + 1
            end

            if locCount == 0 then
                addLog(LOGTYPE_WARNING, 'skip monster %s on map %s because there is no valid gen location', genList.name, getMapName())
                table.insert(badNameKeyList, i)
            end
        end
    end

    for _, badNameKey in pairs(badNameKeyList) do
        monGenList[badNameKey] = nil
    end

    return coroutine.create(function()
        while true do
            local currTime = getTime()
            for _, genList in pairs(monGenList) do
                for _, locInfo in pairs(genList.loc) do
                    if currTime > locInfo.lastUpdateTime + locInfo.time * 1000 then
                        -- seems lua remove a key is pretty complicated
                        -- see: https://stackoverflow.com/questions/12394841/safely-remove-items-from-an-array-table-while-iterating
                        local deadKeyList = {}
                        for ikey, uidString in pairs(locInfo.uidList) do
                            if not isUIDAlive(uidString) then
                                table.insert(deadKeyList, ikey)
                            end
                        end

                        for _, ikey in pairs(deadKeyList) do
                            locInfo.uidList[ikey] = nil
                        end

                        local aliveMonCount = 0
                        for _, _ in pairs(locInfo.uidList) do
                            aliveMonCount = aliveMonCount + 1
                        end

                        -- density: how many grids needs per monster
                        -- this is not exact, some monster can create more monsters
                        local monsterMaxDensity = 8
                        local regionMaxCount = math.max(1, math.min(math.floor(locInfo.gridCount / monsterMaxDensity), locInfo.count))
                        local needCount = regionMaxCount - aliveMonCount

                        while needCount > 0 do
                            local addX, addY = randGLoc(locInfo.x, locInfo.y, locInfo.w, locInfo.h)
                            local uidString = addMonster(genList.name, addX, addY, true)

                            if uidString ~= nil then
                                table.insert(locInfo.uidList, uidString)
                                needCount = needCount - 1
                            end
                        end
                        locInfo.lastUpdateTime = currTime
                    end
                end
            end
            coroutine.yield()
        end
    end)
end

return addmon
