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
--             return true
--         end
--         server.player.postString(uid, '我没有钥匙，无法进入……')
--         return false
--     end)
--
--     deleteGridTrigger(gridTriggerId)
--
-- a handler is called as handler(uid, x, y) and decides what happens next:
--
--     true    let the player through to wherever the grid leads
--     false   keep the player where they are
--
-- returning nothing counts as false, so a handler that moves the player somewhere else
-- itself (uidMapSwitch) just falls through
--
-- handlers follow the same event paths as an NPC: addGridTrigger() installs the SYS_EPDEF
-- handler for everyone and addUIDGridTrigger() installs the SYS_EPUID handler for one
-- player, EPUID is consulted first, so a quest gates its own player with
-- addUIDGridTrigger() and turns everybody else away with addGridTrigger()

local _RSVD_NAME_EPDEF_gridTriggers = {}  -- gridTriggerId -> handler, everyone on the map
local _RSVD_NAME_EPUID_gridTriggers = {}  -- gridTriggerId -> {uid = ..., handler = ...}, one player

-- normalize the rect args shared by addGridTrigger() and addUIDGridTrigger():
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

-- everyone on this map, installed by the map script
function addGridTrigger(...)
    local args = table.pack(...)
    assertType(args[args.n], 'function')

    local handler = args[args.n]
    args[args.n] = nil
    args.n = args.n - 1

    local gridTriggerId = _RSVD_NAME_allocateGridTriggerId(parseGridTriggerRectList(args))
    _RSVD_NAME_EPDEF_gridTriggers[gridTriggerId] = handler
    return gridTriggerId
end

-- remove a trigger again, by the id addGridTrigger() returned
function deleteGridTrigger(gridTriggerId)
    assertType(gridTriggerId, 'integer')
    _RSVD_NAME_removeGridTriggerId(gridTriggerId)
    _RSVD_NAME_EPDEF_gridTriggers[gridTriggerId] = nil
end

-- one player, installed by a quest through setupMapUIDGridTrigger()
--
-- same rect forms as addGridTrigger(), with a uid in front
function addUIDGridTrigger(uid, ...)
    assertType(uid, 'integer')

    local args = table.pack(...)
    assertType(args[args.n], 'function')

    local handler = args[args.n]
    args[args.n] = nil
    args.n = args.n - 1

    local gridTriggerId = _RSVD_NAME_allocateGridTriggerId(parseGridTriggerRectList(args))
    _RSVD_NAME_EPUID_gridTriggers[gridTriggerId] = { uid = uid, handler = handler }
    return gridTriggerId
end

function deleteUIDGridTrigger(gridTriggerId)
    assertType(gridTriggerId, 'integer')
    _RSVD_NAME_removeGridTriggerId(gridTriggerId)
    _RSVD_NAME_EPUID_gridTriggers[gridTriggerId] = nil
end

-- every gridTriggerId covering (x, y), in install order, straight from the C++ side
function getGridTriggerIDList(x, y)
    return _RSVD_NAME_getGridTriggerIDList(x, y)
end

local function getUIDGridTriggerIDAt(uid, x, y)
    for _, gridTriggerId in ipairs(getGridTriggerIDList(x, y)) do
        local entry = _RSVD_NAME_EPUID_gridTriggers[gridTriggerId]
        if entry and entry.uid == uid then
            return gridTriggerId
        end
    end
    return nil
end

-- the per-player trigger of uid at a grid or any grid in a rect list, quest cleanup helper
function getUIDGridTriggerID(uid, x, y)
    if type(x) == 'table' then
        for _, rect in ipairs(parseGridTriggerRectList(table.pack(x))) do
            for dx = 0, rect.w - 1 do
                for dy = 0, rect.h - 1 do
                    local gridTriggerId = getUIDGridTriggerIDAt(uid, rect.x + dx, rect.y + dy)
                    if gridTriggerId then
                        return gridTriggerId
                    end
                end
            end
        end
        return nil
    end
    return getUIDGridTriggerIDAt(uid, x, y)
end

function hasGridTrigger(x, y)
    return #getGridTriggerIDList(x, y) > 0
end

-- remove every all-players (SYS_EPDEF) trigger covering (x, y), quest cleanup helper
function deleteGridTriggerAt(x, y)
    for _, gridTriggerId in ipairs(getGridTriggerIDList(x, y)) do
        if _RSVD_NAME_EPDEF_gridTriggers[gridTriggerId] then
            deleteGridTrigger(gridTriggerId)
        end
    end
end

-- called from ServerMap::dispatchGridSwitch when a player lands on a triggered grid
function _RSVD_NAME_runGridTrigger(uid, x, y)
    local idList = getGridTriggerIDList(x, y)

    -- per-player (SYS_EPUID) triggers first, the quest's own player
    for _, gridTriggerId in ipairs(idList) do
        local entry = _RSVD_NAME_EPUID_gridTriggers[gridTriggerId]
        if entry and entry.uid == uid then
            if entry.handler(uid, x, y) then
                uidGridMapSwitch(uid, x, y)
            end
            return
        end
    end

    -- then the default (SYS_EPDEF) trigger, everyone on the map
    for _, gridTriggerId in ipairs(idList) do
        local handler = _RSVD_NAME_EPDEF_gridTriggers[gridTriggerId]
        if handler then
            if handler(uid, x, y) then
                uidGridMapSwitch(uid, x, y)
            end
            return
        end
    end

    -- no handler applies to this player, e.g. only other players' per-player triggers sit
    -- on the grid, let the player through rather than trapping them
    uidGridMapSwitch(uid, x, y)
end
