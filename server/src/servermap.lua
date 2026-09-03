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
-- handlers follow the same event paths as an NPC, a map script installs SYS_EPDEF for
-- everyone and a quest installs SYS_EPUID for one player, EPUID is consulted first
--
-- a handler is called as handler(uid, x, y) and decides what happens next:
--
--     true    let the player through to wherever the grid leads
--     false   keep the player where they are
--
-- returning nothing counts as false, so a handler that moves the player somewhere else
-- itself (uidMapSwitch) just falls through
--
--     setGridTrigger(226, 177, function(uid, x, y)
--         if server.player.hasItem(uid, '牢房钥匙', 1) then
--             server.player.postString(uid, '门被打开了！进去看看……')
--             return true
--         end
--         server.player.postString(uid, '我没有钥匙，无法进入……')
--         return false
--     end)

local _RSVD_NAME_EPDEF_gridTriggers = {}
local _RSVD_NAME_EPUID_gridTriggers = {}

-- how many handlers sit on a grid, the C++ side only needs to know whether the automatic
-- switch has been taken over, so it is kept in step with this count
local _RSVD_NAME_gridTriggerCount = {}

local function gridKey(x, y)
    assertType(x, 'integer')
    assertType(y, 'integer')
    return x .. ',' .. y
end

local function addGridTriggerRef(x, y)
    local key = gridKey(x, y)
    local count = (_RSVD_NAME_gridTriggerCount[key] or 0) + 1

    _RSVD_NAME_gridTriggerCount[key] = count
    if count == 1 then
        setGridSwitchTrigger(x, y, 1, 1)
    end
end

local function removeGridTriggerRef(x, y)
    local key = gridKey(x, y)
    local count = _RSVD_NAME_gridTriggerCount[key] or 0

    if count <= 0 then
        return
    end

    count = count - 1
    _RSVD_NAME_gridTriggerCount[key] = (count > 0) and count or nil

    if count == 0 then
        clearGridSwitchTrigger(x, y, 1, 1)
    end
end

-- everyone on this map, installed by the map script
function setGridTrigger(x, y, handler)
    assertType(handler, 'function')
    local key = gridKey(x, y)

    if _RSVD_NAME_EPDEF_gridTriggers[key] == nil then
        addGridTriggerRef(x, y)
    end
    _RSVD_NAME_EPDEF_gridTriggers[key] = handler
end

function deleteGridTrigger(x, y)
    local key = gridKey(x, y)

    if _RSVD_NAME_EPDEF_gridTriggers[key] ~= nil then
        _RSVD_NAME_EPDEF_gridTriggers[key] = nil
        removeGridTriggerRef(x, y)
    end
end

-- one player, installed by a quest through setupMapGridTrigger()
function setUIDGridTrigger(uid, x, y, handler)
    assertType(uid, 'integer')
    assertType(handler, 'function')

    local key = gridKey(x, y)
    if _RSVD_NAME_EPUID_gridTriggers[key] == nil then
        _RSVD_NAME_EPUID_gridTriggers[key] = {}
    end

    if _RSVD_NAME_EPUID_gridTriggers[key][uid] == nil then
        addGridTriggerRef(x, y)
    end
    _RSVD_NAME_EPUID_gridTriggers[key][uid] = handler
end

function deleteUIDGridTrigger(uid, x, y)
    assertType(uid, 'integer')
    local key = gridKey(x, y)

    if _RSVD_NAME_EPUID_gridTriggers[key] == nil then
        return
    end

    if _RSVD_NAME_EPUID_gridTriggers[key][uid] ~= nil then
        _RSVD_NAME_EPUID_gridTriggers[key][uid] = nil
        removeGridTriggerRef(x, y)

        if next(_RSVD_NAME_EPUID_gridTriggers[key]) == nil then
            _RSVD_NAME_EPUID_gridTriggers[key] = nil
        end
    end
end

function hasGridTrigger(x, y)
    return (_RSVD_NAME_gridTriggerCount[gridKey(x, y)] or 0) > 0
end

-- called from ServerMap::dispatchGridSwitch when a player lands on a triggered grid
function _RSVD_NAME_runGridTrigger(uid, x, y)
    local key = gridKey(x, y)
    local handler = nil

    if _RSVD_NAME_EPUID_gridTriggers[key] then
        handler = _RSVD_NAME_EPUID_gridTriggers[key][uid]
    end

    if handler == nil then
        handler = _RSVD_NAME_EPDEF_gridTriggers[key]
    end

    -- the count and the handler tables went out of step, let the player through rather
    -- than trapping them on a grid nobody owns
    if handler == nil then
        uidGridMapSwitch(uid, x, y)
        return
    end

    if handler(uid, x, y) then
        uidGridMapSwitch(uid, x, y)
    end
end
