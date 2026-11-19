-- run func on its own lua thread in this map, returns the key to cancel it by
--
-- same shape as runQuestThread: pause() inside the thread to delay the rest of it, and
-- closeThread(key) cancels the pause and the thread with it
function runMapThread(func)
    assertType(func, 'function')

    local key = rollKey()
    runThread(key, func)
    return key
end

-- grid triggers
--
-- a grid listed in the map record's switchList normally sends a player straight to the
-- next map. install a trigger on that grid and a script gets the decision instead: check an
-- item, a finished quest, a level, a gender, then either let the player through or tell them
-- why the door will not open
--
-- every trigger owns a rect array (each {x, y, w, h}) and gets a unique gridTriggerId from
-- the C++ side, which keeps gridTriggerId -> rects in m_gridTriggerList and
-- grid -> [gridTriggerId] in each grid. a grid with any trigger stops auto-switching and
-- the script decides instead
--
-- addGridTrigger() creates the trigger, binds the handler under the returned id and hands
-- the id back, deleteGridTrigger() removes it again:
--
--     local gridTriggerId = addGridTrigger(226, 177, function(uid, x, y)
--         if server.player.hasItem(uid, '牢房钥匙', 1) then
--             server.player.postString(uid, '门被打开了！进去看看……')
--             uidGridMapSwitch(uid, x, y)
--             return
--         end
--         server.player.postString(uid, '我没有钥匙，无法进入……')
--     end)
--
--     deleteGridTrigger(gridTriggerId)
--
-- a handler is called as handler(uid, x, y), a grid it runs on never sends the player on by
-- itself: the handler calls uidGridMapSwitch(uid, x, y) to do that, or moves the player
-- anywhere else, or keeps them where they are
--
-- handlers follow the same event paths as an NPC: a SYS_EPDEF trigger is for everyone and a
-- SYS_EPUID trigger for one player
--
--     addGridTrigger()                    SYS_EPDEF, installed by the map script, belongs to no quest
--     addQuestGridTrigger()               SYS_EPDEF, installed by a quest
--     addUIDGridTrigger()                 SYS_EPUID, installed by a quest for one player
--     _RSVD_NAME_setUIDGridTrigger()      SYS_EPUID with a name, a quest installs it again by the name to replace it
--
-- every trigger records its type, its player and its quest, see getGridTriggerInfo()
-- deleteGridTrigger() removes a trigger of any kind
--
-- which handlers run is decided per quest, see _RSVD_NAME_runGridTrigger(): a player's own
-- SYS_EPUID triggers of a quest replace the SYS_EPDEF ones of that quest, so a quest gates
-- its own player with a SYS_EPUID trigger and turns everybody else away with a SYS_EPDEF one
-- a SYS_EPUID handler returning exactly true retires its trigger

-- gridTriggerId -> {type = SYS_EPDEF or SYS_EPUID, uid = the player of a SYS_EPUID trigger, quest = the quest that installed it, handler = handler}
-- a named SYS_EPUID trigger has name, and version and questUID if its quest saves it, see _RSVD_NAME_setUIDGridTrigger()
local _RSVD_NAME_gridTriggers = {}

-- uid -> questName -> name -> gridTriggerId, the SYS_EPUID triggers a quest named, see _RSVD_NAME_setUIDGridTrigger()
local _RSVD_NAME_EPUID_namedGridTriggers = {}

-- normalize the rect args shared by the grid trigger add functions:
--
--     x, y                   a 1x1 rect
--     x, y, w, h             one rect
--     rectList               array of {x = ..., y = ..., w = ..., h = ...}, w/h default to 1
--
-- argList is a table.pack()-ed slice that holds the rect args only, the handler is never
-- part of it
local function parseRectList(argList)
    local rectList = nil

    if argList.n == 1 and type(argList[1]) == 'table' then
        rectList = argList[1]

    elseif argList.n == 2 or argList.n == 4 then
        rectList = {{ x = argList[1], y = argList[2], w = argList[3] or 1, h = argList[4] or 1 }}

    else
        fatalPrintf('Invalid rect arguments, expect (x, y), (x, y, w, h) or a rect array')
    end

    local result = {}
    for _, rect in ipairs(rectList) do
        assertType(rect, 'table')
        local x = rect.x or rect[1]
        local y = rect.y or rect[2]
        local w = rect.w or rect[3] or 1
        local h = rect.h or rect[4] or 1
        assertType(x, 'integer')
        assertType(y, 'integer')
        assertType(w, 'integer')
        assertType(h, 'integer')
        result[#result + 1] = { x = x, y = y, w = w, h = h }
    end
    assert(#result > 0, 'expect at least one rect')
    return result
end

-- the handler is the last of the args, the rect args come before it
local function addGridTriggerRecord(record, ...)
    local args = table.pack(...)
    assertType(args[args.n], 'function')

    record.handler = args[args.n]
    args[args.n] = nil
    args.n = args.n - 1

    local gridTriggerId = _RSVD_NAME_allocateGridTriggerId(parseRectList(args))
    _RSVD_NAME_gridTriggers[gridTriggerId] = record
    return gridTriggerId
end

-- everyone on this map, installed by the map script
function addGridTrigger(...)
    return addGridTriggerRecord({type = SYS_EPDEF}, ...)
end

-- everyone on this map, installed by quest questName, see setupMapGridTrigger() in quest.lua
function addQuestGridTrigger(questName, ...)
    assertType(questName, 'string')
    return addGridTriggerRecord({type = SYS_EPDEF, quest = questName}, ...)
end

-- one player, installed by quest questName, a quest names its triggers, see _RSVD_NAME_setUIDGridTrigger()
function addUIDGridTrigger(uid, questName, ...)
    assertType(uid, 'integer')
    assertType(questName, 'string')
    return addGridTriggerRecord({type = SYS_EPUID, uid = uid, quest = questName}, ...)
end

-- installs the trigger name of quest questName for uid, or replaces it, in one call with no yield, the grid is never without it
-- version and questUID come from the quest, a retire is reported to questUID with version, see _RSVD_NAME_runGridTrigger()
-- a trigger the quest doesn't save, i.e. on a map copy, comes without them and is never reported
function _RSVD_NAME_setUIDGridTrigger(uid, questName, name, version, questUID, ...)
    assertType(name, 'string')
    assertType(version, 'integer', 'nil')
    assertType(questUID, 'integer', 'nil')

    local oldTriggerId = ((_RSVD_NAME_EPUID_namedGridTriggers[uid] or {})[questName] or {})[name]
    local gridTriggerId = addUIDGridTrigger(uid, questName, ...)

    local record = _RSVD_NAME_gridTriggers[gridTriggerId]
    record.name = name
    record.version = version
    record.questUID = questUID

    if not _RSVD_NAME_EPUID_namedGridTriggers[uid] then
        _RSVD_NAME_EPUID_namedGridTriggers[uid] = {}
    end

    if not _RSVD_NAME_EPUID_namedGridTriggers[uid][questName] then
        _RSVD_NAME_EPUID_namedGridTriggers[uid][questName] = {}
    end

    _RSVD_NAME_EPUID_namedGridTriggers[uid][questName][name] = gridTriggerId
    if oldTriggerId then
        deleteGridTrigger(oldTriggerId)
    end
    return gridTriggerId
end

-- deletes the trigger name of quest questName for uid, if version is given only the copy of that version
-- a delete of an old copy then never hits a newer one installed by the same name
function _RSVD_NAME_deleteUIDGridTrigger(uid, questName, name, version)
    assertType(uid, 'integer')
    assertType(questName, 'string')
    assertType(name, 'string')
    assertType(version, 'integer', 'nil')

    local gridTriggerId = ((_RSVD_NAME_EPUID_namedGridTriggers[uid] or {})[questName] or {})[name]
    if gridTriggerId and ((version == nil) or (_RSVD_NAME_gridTriggers[gridTriggerId].version == version)) then
        deleteGridTrigger(gridTriggerId)
    end
end

-- remove a trigger of any kind, by the id its add function returned
function deleteGridTrigger(gridTriggerId)
    assertType(gridTriggerId, 'integer')
    _RSVD_NAME_removeGridTriggerId(gridTriggerId)

    local record = _RSVD_NAME_gridTriggers[gridTriggerId]
    if not record then
        return
    end

    _RSVD_NAME_gridTriggers[gridTriggerId] = nil
    if record.type == SYS_EPUID then
        -- a replace installs the new trigger before it deletes the old one, the name stays with the new one
        local namedList = _RSVD_NAME_EPUID_namedGridTriggers[record.uid]
        if record.name and namedList and namedList[record.quest] and (namedList[record.quest][record.name] == gridTriggerId) then
            namedList[record.quest][record.name] = nil
            if next(namedList[record.quest]) == nil then
                namedList[record.quest] = nil
            end
            if next(namedList) == nil then
                _RSVD_NAME_EPUID_namedGridTriggers[record.uid] = nil
            end
        end
    end
end

-- {type, uid, quest, name, version} of a trigger, see the add functions, nil if there is no such trigger
function getGridTriggerInfo(gridTriggerId)
    assertType(gridTriggerId, 'integer')

    local record = _RSVD_NAME_gridTriggers[gridTriggerId]
    if record then
        return {type = record.type, uid = record.uid, quest = record.quest, name = record.name, version = record.version}
    end
end

-- every gridTriggerId covering (x, y), in install order, straight from the C++ side
function getGridTriggerIDList(x, y)
    return _RSVD_NAME_getGridTriggerIDList(x, y)
end

function hasGridTrigger(x, y)
    return #getGridTriggerIDList(x, y) > 0
end

-- called from ServerMap::dispatchGridSwitch when a player lands on a triggered grid
--
-- per quest: the player's SYS_EPUID triggers of the quest run, else its SYS_EPDEF ones
-- quests don't silence each other, and a SYS_EPDEF trigger of the map script belongs to no quest, it always runs
--
-- all chosen handlers run in install order, from the list taken before the first one, a handler can add and delete triggers
-- a trigger deleted meanwhile doesn't run, a new one doesn't either
-- a raising handler is logged, the others still run
-- a SYS_EPUID handler returning exactly true retires its trigger, other results are ignored
--
-- no trigger applies to the player, i.e. only other players' SYS_EPUID ones are on the grid: it sends the player on as a grid without triggers does
function _RSVD_NAME_runGridTrigger(uid, x, y)
    local idList = getGridTriggerIDList(x, y)

    local uidQuests = {}
    for _, gridTriggerId in ipairs(idList) do
        local record = _RSVD_NAME_gridTriggers[gridTriggerId]
        if record and (record.type == SYS_EPUID) and (record.uid == uid) then
            uidQuests[record.quest] = true
        end
    end

    local runList = {}
    for _, gridTriggerId in ipairs(idList) do
        local record = _RSVD_NAME_gridTriggers[gridTriggerId]
        if record then
            if record.type == SYS_EPUID then
                if record.uid == uid then
                    table.insert(runList, gridTriggerId)
                end
            elseif (record.quest == nil) or (not uidQuests[record.quest]) then
                table.insert(runList, gridTriggerId)
            end
        end
    end

    if #runList == 0 then
        uidGridMapSwitch(uid, x, y)
        return
    end

    for _, gridTriggerId in ipairs(runList) do
        local record = _RSVD_NAME_gridTriggers[gridTriggerId]
        if record then
            local ok, result = xpcall(record.handler, debug.traceback, uid, x, y)
            if not ok then
                addLog(LOGTYPE_WARNING, 'Grid trigger %d of quest %s raised for player %d at (%d, %d)', gridTriggerId, tostring(record.quest), uid, x, y)
                for line in tostring(result):gmatch('[^\n]+') do
                    addLog(LOGTYPE_WARNING, '%s', line)
                end

            elseif (record.type == SYS_EPUID) and (result == true) then
                deleteGridTrigger(gridTriggerId)

                -- the quest drops a notice whose version isn't its last write of the name, a replace made meanwhile wins
                if record.version and record.questUID then
                    runMapThread(function()
                        uidRemoteCall(record.questUID, uid, record.name, record.version, [[ _RSVD_NAME_retireQuestGridTrigger(...) ]])
                    end)
                end
            end
        end
    end
end

-- safe zones
--
-- inside a safe zone, no player can damage others, or anything a player controls
-- and nobody standing in one can deal such damage out of it, see Player::canDamageTarget()
-- and BattleObject::safeZoneBlocksAttack()
--
-- a map script, or a quest through uidRemoteCall() to the map, names every zone it installs
-- a zone is any number of rects and circles, args after the name are:
--
--     (none)                 the whole map
--     x, y                   one grid
--     x, y, r                a circle, r > 0
--     x, y, w, h             a rect
--     shape                  one shape table
--     shapeList              array of shape tables, rects and circles can mix
--
-- a shape table takes exactly one of these key sets, anything else is an error:
--
--     {x = ..., y = ...}                     one grid
--     {x = ..., y = ..., r = ...}            a circle
--     {x = ..., y = ..., w = ...}            a rect, h = 1
--     {x = ..., y = ..., h = ...}            a rect, w = 1
--     {x = ..., y = ..., w = ..., h = ...}   a rect
--
-- updating a zone replaces all its shapes with the same kind of args, so a zone can move, grow, shrink or change shape:
--
--     addSafeZone('dungeon')                                                 -- the whole map
--     addSafeZone('town', {{x = 300, y = 260, w = 80, h = 60}, {x = 400, y = 290, r = 20}})
--     addSafeZone('gate', 330, 330)                                          -- one grid
--     updateSafeZone('gate', 330, 330, 5)                                    -- a circle, replaces the grid above
--     deleteSafeZone('gate')
--
-- adding an existing name or updating an unknown name is an error, deleting an unknown name is a no-op
-- a grid is safe if any shape of any zone covers it, isSafeGrid(x, y) answers that
-- zones only live in memory, the map script adds them again whenever the map loads

local function parseSafeZoneShape(shape)
    assertType(shape, 'table')
    for k, _ in pairs(shape) do
        if not (k == 'x' or k == 'y' or k == 'r' or k == 'w' or k == 'h') then
            fatalPrintf('Invalid safe zone shape key: %s', tostring(k))
        end
    end

    assertType(shape.x, 'integer')
    assertType(shape.y, 'integer')

    if shape.r ~= nil then
        if shape.w ~= nil or shape.h ~= nil then
            fatalPrintf('Invalid safe zone shape: r can not go with w or h')
        end

        assertType(shape.r, 'integer')
        assert(shape.r > 0, 'safe zone circle radius must be positive')
        return {x = shape.x, y = shape.y, r = shape.r}, true
    end

    local w = shape.w or 1
    local h = shape.h or 1

    assertType(w, 'integer')
    assertType(h, 'integer')
    assert(w > 0 and h > 0, 'safe zone rect size must be positive')
    return {x = shape.x, y = shape.y, w = w, h = h}, false
end

-- argList is a table.pack()-ed slice of the args after the zone name
-- returns the rect list and the circle list
local function parseSafeZoneArgs(argList)
    local shapeList = nil

    if argList.n == 0 then
        local mapW, mapH = getMapSize()
        shapeList = {{x = 0, y = 0, w = mapW, h = mapH}}

    elseif argList.n == 1 then
        assertType(argList[1], 'table')
        if argList[1].x ~= nil then
            shapeList = {argList[1]}
        else
            shapeList = argList[1]
            local count = 0
            for k, _ in pairs(shapeList) do
                if math.type(k) ~= 'integer' or k < 1 or k > #shapeList then
                    fatalPrintf('Invalid safe zone shape list key: %s', tostring(k))
                end
                count = count + 1
            end
            assert(count > 0 and count == #shapeList, 'expect a non-empty safe zone shape array')
        end

    elseif argList.n >= 2 and argList.n <= 4 then
        for i = 1, argList.n do
            assertType(argList[i], 'integer')
        end

        if argList.n == 2 then
            shapeList = {{x = argList[1], y = argList[2]}}
        elseif argList.n == 3 then
            shapeList = {{x = argList[1], y = argList[2], r = argList[3]}}
        else
            shapeList = {{x = argList[1], y = argList[2], w = argList[3], h = argList[4]}}
        end

    else
        fatalPrintf('Invalid safe zone arguments, expect (), (x, y), (x, y, r), (x, y, w, h), a shape or a shape array')
    end

    local rectList, circleList = {}, {}
    for _, shape in ipairs(shapeList) do
        local parsed, isCircle = parseSafeZoneShape(shape)
        table.insert(isCircle and circleList or rectList, parsed)
    end
    return rectList, circleList
end

-- name -> safeZoneId
local _RSVD_NAME_safeZones = {}

function addSafeZone(name, ...)
    assertType(name, 'string')
    if _RSVD_NAME_safeZones[name] then
        fatalPrintf('Safe zone already exists: %s', name)
    end
    _RSVD_NAME_safeZones[name] = _RSVD_NAME_allocateSafeZoneId(parseSafeZoneArgs(table.pack(...)))
end

function updateSafeZone(name, ...)
    assertType(name, 'string')
    local safeZoneId = _RSVD_NAME_safeZones[name]
    if not safeZoneId then
        fatalPrintf('Safe zone does not exist: %s', name)
    end
    assert(_RSVD_NAME_updateSafeZoneId(safeZoneId, parseSafeZoneArgs(table.pack(...))), 'safe zone lost in map: ' .. name)
end

function deleteSafeZone(name)
    assertType(name, 'string')
    local safeZoneId = _RSVD_NAME_safeZones[name]
    if safeZoneId then
        _RSVD_NAME_safeZones[name] = nil
        _RSVD_NAME_removeSafeZoneId(safeZoneId)
    end
end

function hasSafeZone(name)
    assertType(name, 'string')
    return _RSVD_NAME_safeZones[name] ~= nil
end
