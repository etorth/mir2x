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
-- a grid listed in the map record's mapSwitchList normally sends a player straight to the
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
--     addGridTrigger()         SYS_EPDEF, installed by the map script, belongs to no quest
--     addQuestGridTrigger()    SYS_EPDEF, installed by a quest
--     addUIDGridTrigger()      SYS_EPUID, installed by a quest for one player
--
-- every trigger records its type, its player and its quest, see getGridTriggerInfo()
-- deleteGridTrigger() removes a trigger of any kind
--
-- which handlers run is decided per quest, see _RSVD_NAME_runGridTrigger(): a player's own
-- SYS_EPUID triggers of a quest replace the SYS_EPDEF ones of that quest, so a quest gates
-- its own player with a SYS_EPUID trigger and turns everybody else away with a SYS_EPDEF one
-- a SYS_EPUID handler returning exactly true retires its trigger

-- gridTriggerId -> {type = SYS_EPDEF or SYS_EPUID, uid = the player of a SYS_EPUID trigger, quest = the quest that installed it, handler = handler}
local _RSVD_NAME_gridTriggers = {}

-- uid -> questName -> {gridTriggerId = true, ...}, the SYS_EPUID triggers of each player by quest
local _RSVD_NAME_EPUID_questGridTriggers = {}

-- normalize the rect args shared by the add functions:
--
--     x, y                   a 1x1 rect
--     x, y, w, h             one rect
--     rectList               array of {x = ..., y = ..., w = ..., h = ...}, w/h default to 1
--
-- argList is a table.pack()-ed slice that holds the rect args only, the handler is never
-- part of it
local function parseGridTriggerRectList(argList)
    local rectList = nil

    if argList.n == 1 and type(argList[1]) == 'table' then
        rectList = argList[1]

    elseif argList.n == 2 or argList.n == 4 then
        rectList = {{ x = argList[1], y = argList[2], w = argList[3] or 1, h = argList[4] or 1 }}

    else
        fatalPrintf('Invalid rect arguments to grid trigger, expect (x, y), (x, y, w, h) or a rect array')
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
    assert(#result > 0, 'a grid trigger needs at least one rect')
    return result
end

-- the handler is the last of the args, the rect args come before it
local function addGridTriggerRecord(record, ...)
    local args = table.pack(...)
    assertType(args[args.n], 'function')

    record.handler = args[args.n]
    args[args.n] = nil
    args.n = args.n - 1

    local gridTriggerId = _RSVD_NAME_allocateGridTriggerId(parseGridTriggerRectList(args))
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

-- one player, installed by quest questName
-- the quest removes them in bulk by _RSVD_NAME_clearQuestUIDGridTrigger() once the quest is done
function addUIDGridTrigger(uid, questName, ...)
    assertType(uid, 'integer')
    assertType(questName, 'string')

    local gridTriggerId = addGridTriggerRecord({type = SYS_EPUID, uid = uid, quest = questName}, ...)

    local uidGridTriggerList = _RSVD_NAME_EPUID_questGridTriggers[uid]
    if not uidGridTriggerList then
        uidGridTriggerList = {}
        _RSVD_NAME_EPUID_questGridTriggers[uid] = uidGridTriggerList
    end

    local questGridTriggerList = uidGridTriggerList[questName]
    if not questGridTriggerList then
        questGridTriggerList = {}
        uidGridTriggerList[questName] = questGridTriggerList
    end

    questGridTriggerList[gridTriggerId] = true
    return gridTriggerId
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
        local questTriggerList = _RSVD_NAME_EPUID_questGridTriggers[record.uid]
        if questTriggerList and questTriggerList[record.quest] then
            questTriggerList[record.quest][gridTriggerId] = nil
            if next(questTriggerList[record.quest]) == nil then
                questTriggerList[record.quest] = nil
            end
            if next(questTriggerList) == nil then
                _RSVD_NAME_EPUID_questGridTriggers[record.uid] = nil
            end
        end
    end
end

function _RSVD_NAME_clearQuestUIDGridTrigger(uid, questName)
    assertType(uid, 'integer')
    assertType(questName, 'string')

    local questTriggerList = _RSVD_NAME_EPUID_questGridTriggers[uid]
    local idList = questTriggerList and questTriggerList[questName]
    if not idList then
        return
    end

    local ids = {}
    for gridTriggerId in pairs(idList) do
        ids[#ids + 1] = gridTriggerId
    end

    for _, gridTriggerId in ipairs(ids) do
        deleteGridTrigger(gridTriggerId)
    end
end

-- {type, uid, quest} of a trigger, see the add functions, nil if there is no such trigger
function getGridTriggerInfo(gridTriggerId)
    assertType(gridTriggerId, 'integer')

    local record = _RSVD_NAME_gridTriggers[gridTriggerId]
    if record then
        return {type = record.type, uid = record.uid, quest = record.quest}
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
            end
        end
    end
end
