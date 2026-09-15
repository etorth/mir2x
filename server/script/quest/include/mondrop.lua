-- quest rewards that come off a monster's corpse
--
-- legacy Envir kept these in QuestDiary/NQ_*/MonQuest/Nm_*.txt: a script run when a
-- monster dies, gated on the killer's quest flags, that hands over a quest item and
-- moves the quest along
--
-- the trigger lives on the quest actor instead of on the player, so it keeps working
-- after a relog, and it reads the player's persisted quest state on every kill

local mondrop = {}

-- legacy ramp, see Nm_Oma.txt and friends
--
-- the counter jumps straight to 3 on the first kill and climbs by one after that, and the
-- last two steps of a long ramp are a coin flip each so the drop doesn't land on a
-- predictable kill, a short ramp climbs straight up
local function bumpKillCount(count, kills)
    if count < 3 then
        return 3
    end

    if kills >= 5 and count > kills - 2 and math.random(2) ~= 1 then
        return count
    end
    return count + 1
end

-- accept 'name', {'name', count}, or a list of either
local function asItemList(arg)
    if arg == nil then
        return {}
    end

    if type(arg) == 'string' then
        return {{arg, 1}}
    end

    assertType(arg, 'table')
    if type(arg[1]) == 'string' and (arg[2] == nil or math.type(arg[2]) == 'integer') then
        return {{arg[1], arg[2] or 1}}
    end

    local result = {}
    for _, v in ipairs(arg) do
        for _, item in ipairs(asItemList(v)) do
            table.insert(result, item)
        end
    end
    return result
end

local function asNameList(arg)
    assertType(arg, 'string', 'table')
    if type(arg) == 'string' then
        return {arg}
    end

    for _, v in ipairs(arg) do
        assertType(v, 'string')
    end
    return arg
end

-- returns true when the drop fired, so a monster carrying more than one drop only ever
-- hands over the first one that is live
local function onMap(playerUID, mapList)
    if #mapList == 0 then
        return true
    end

    local mapName = server.player.getMapName(playerUID)
    for _, v in ipairs(mapList) do
        if v == mapName then
            return true
        end
    end
    return false
end

local function runDrop(playerUID, drop)
    -- legacy keyed its MonDie hooks on the map the monster died on, see Envir/MapQuest.txt
    if not onMap(playerUID, drop.map) then
        return false
    end

    for _, item in ipairs(drop.need) do
        if not server.player.hasItem(playerUID, item[1], item[2]) then
            return false
        end
    end

    -- can't hand out a second copy of something the player is still carrying
    if drop.once then
        for _, item in ipairs(drop.give) do
            if server.player.hasItem(playerUID, item[1], item[2]) then
                return false
            end
        end
    end

    if drop.chance > 1 and math.random(drop.chance) ~= 1 then
        return false
    end

    if drop.kills > 1 then
        local count = bumpKillCount(dbGetQuestVar(playerUID, drop.counter) or 0, drop.kills)
        if count <= drop.kills then
            dbSetQuestVar(playerUID, drop.counter, count)
            return false
        end
        dbSetQuestVar(playerUID, drop.counter, nil)
    end

    for _, item in ipairs(drop.take) do
        server.player.removeItem(playerUID, item[1], item[2])
    end

    for _, item in ipairs(drop.give) do
        server.player.addItem(playerUID, item[1], item[2])
    end

    if drop.say then
        server.player.postString(playerUID, drop.say)
    end

    if drop.moveTo then
        -- a bare map name is legacy's `map X`, which drops the player anywhere walkable on it
        if #drop.moveTo == 1 then
            local mapUID = loadBaseMap(drop.moveTo[1])
            if mapUID then
                local x, y = uidRemoteCall(mapUID, [[ return getRandLoc() ]])
                server.player.spaceMove(playerUID, drop.moveTo[1], x, y)
            end
        else
            server.player.spaceMove(playerUID, table.unpack(drop.moveTo))
        end
    end

    -- last, it can clear the state this drop is gated on
    if drop.setState then
        setQuestState{uid = playerUID, state = drop.setState}
    end
    return true
end

-- the dropList shape addDropTrigger takes
--
--     {
--         {
--             monster  = '千年毒蛇',          -- name, or list of names
--             map      = '沃玛神殿_D022',      -- optional, only on these maps
--             kills    = 10,                 -- optional, roughly how many kills it takes
--             chance   = 2,                  -- optional, 1/chance per kill instead
--             once     = true,               -- optional, don't hand out a second copy
--             need     = '角笛',              -- optional, must be carrying this
--             give     = '千年毒蛇胆汁',       -- optional, 'name' / {'name', count} / list
--             take     = '角笛',              -- optional, same shapes as give
--             setState = 'quest_got_gall',   -- optional, state to move to afterwards
--             moveTo   = {'D001', 303, 70},  -- optional, where to put the player, or just
--                                            -- {'D001'} for anywhere on it, legacy's `map X`
--             say      = '...',              -- optional, message to the player
--         },
--     }
--
-- validates dropList and indexes it by monster id so a kill only has to walk the drops that
-- could actually fire
local function buildDropListByMonster(dropList)
    assertType(dropList, 'table')

    local dropListByMonster = {}

    for _, drop in ipairs(dropList) do
        assertType(drop, 'table')
        assertType(drop.kills, 'integer', 'nil')
        assertType(drop.chance, 'integer', 'nil')
        assertType(drop.once, 'boolean', 'nil')
        assertType(drop.say, 'string', 'nil')
        assertType(drop.moveTo, 'table', 'nil')
        assertType(drop.setState, 'string', 'nil')

        if drop.kills and drop.chance then
            fatalPrintf('Monster drop takes kills or chance, not both')
        end

        local monsterNameList = asNameList(drop.monster)

        local parsed =
        {
            map      = drop.map and asNameList(drop.map) or {},
            need     = asItemList(drop.need),
            give     = asItemList(drop.give),
            take     = asItemList(drop.take),
            kills    = drop.kills or 1,
            chance   = drop.chance or 1,
            once     = drop.once,
            say      = drop.say,
            moveTo   = drop.moveTo,
            setState = drop.setState,

            -- keyed on the monsters so the count survives a restart, legacy used one
            -- counter per monster script
            counter  = drop.counter or ('mondrop_' .. table.concat(monsterNameList, '_')),
        }

        for _, mapName in ipairs(parsed.map) do
            if getMapID(mapName) <= 0 then
                fatalPrintf('Monster drop refers to unknown map %s', mapName)
            end
        end

        if parsed.setState and (parsed.setState ~= SYS_DONE) and (not hasQuestState(parsed.setState)) then
            fatalPrintf('Monster drop moves to unknown quest state %s', parsed.setState)
        end

        for _, list in ipairs({parsed.need, parsed.give, parsed.take}) do
            for _, item in ipairs(list) do
                if getItemID(item[1]) <= 0 then
                    fatalPrintf('Monster drop refers to unknown item %s', item[1])
                end
            end
        end

        for _, monsterName in ipairs(monsterNameList) do
            local monsterID = getMonsterID(monsterName)
            if monsterID <= 0 then
                fatalPrintf('Monster drop refers to unknown monster %s', monsterName)
            end

            if not dropListByMonster[monsterID] then
                dropListByMonster[monsterID] = {}
            end
            table.insert(dropListByMonster[monsterID], parsed)
        end
    end

    return dropListByMonster
end

-- call this from inside the quest_xxx state the drop is meant to be live in (not once at quest
-- script load time), the trigger lives in the player's own VM so a kill costs the killer a cheap
-- local table lookup instead of a remote call to the quest actor for every single kill on the
-- server, only a real match pays for the round trip back here to run runDrop
--
--     quest_wait_kill = function(uid, args)
--         local trigger = mondrop.addDropTrigger(uid,
--         {
--             {
--                 monster  = '半兽人',
--                 state    = 'quest_wait_kill',
--                 setState = 'quest_done',
--             },
--         },
--         {
--             timeout   = 100 * 1000,
--             onTimeout = function()
--                 postString(uid, '时间到了，任务失败。')
--                 setQuestState{uid=uid, state='quest_failed'}
--             end,
--         })
--     end,
--
-- returns a handle for mondrop.deleteDropTrigger, opts is optional:
--     opts.timeout   -- optional, milliseconds, auto-removes the trigger after this long
--     opts.onTimeout -- optional, called (in this quest's own VM) when the timeout fires
local _RSVD_NAME_activeDropCalls = {}
local _RSVD_NAME_dropCallSeqID = 0

local function _RSVD_NAME_removePlayerTrigger(uid, triggerPath)
    uidRemoteCall(uid, triggerPath,
    [[
        local triggerPath = ...
        deleteTrigger(triggerPath)
    ]])
end

-- called from the player's own VM once a kill matches, must stay reachable through the module
-- table since it runs from a remote-call code string with no upvalue access, but it is not part
-- of mondrop's supported interface, callers should only use addDropTrigger/deleteDropTrigger
function mondrop._runDropOnKill(playerUID, callID, monsterID)
    local call = _RSVD_NAME_activeDropCalls[callID]
    if not call then
        return
    end

    for _, drop in ipairs(call.dropListByMonster[monsterID] or {}) do
        if runDrop(playerUID, drop) then
            _RSVD_NAME_activeDropCalls[callID] = nil
            if call.timerKey then
                closeThread(call.timerKey)
            end
            _RSVD_NAME_removePlayerTrigger(playerUID, call.triggerPath)
            return
        end
    end
end

function mondrop.addDropTrigger(uid, dropList, opts)
    assertType(uid, 'integer')
    assertType(dropList, 'table')
    assertType(opts, 'table', 'nil')

    if opts then
        assertType(opts.timeout, 'integer', 'nil')
        assertType(opts.onTimeout, 'function', 'nil')
    end

    local dropListByMonster = buildDropListByMonster(dropList)
    local monsterIDList = {}
    for monsterID in pairs(dropListByMonster) do
        table.insert(monsterIDList, monsterID)
    end

    _RSVD_NAME_dropCallSeqID = _RSVD_NAME_dropCallSeqID + 1
    local callID = _RSVD_NAME_dropCallSeqID
    local triggerPath = uidRemoteCall(uid, getUID(), callID, monsterIDList,
    [[
        local questUID, callID, monsterIDList = ...
        local monsterIDSet = {}
        for _, monsterID in ipairs(monsterIDList) do
            monsterIDSet[monsterID] = true
        end

        return addTrigger(SYS_ON_KILL, function(monsterUID)
            local monsterID = getMonsterID(monsterUID)
            if monsterIDSet[monsterID] then
                uidRemoteCall(questUID, getUID(), callID, monsterID,
                [=[
                    local playerUID, callID, monsterID = ...
                    require('quest.include.mondrop')._runDropOnKill(playerUID, callID, monsterID)
                ]=])
            end
        end)
    ]])

    _RSVD_NAME_activeDropCalls[callID] =
    {
        uid               = uid,
        dropListByMonster = dropListByMonster,
        triggerPath       = triggerPath,
    }

    if opts and opts.timeout then
        _RSVD_NAME_activeDropCalls[callID].timerKey = runQuestThread(function()
            pause(opts.timeout)

            local call = _RSVD_NAME_activeDropCalls[callID]
            if not call then
                return
            end

            _RSVD_NAME_activeDropCalls[callID] = nil
            _RSVD_NAME_removePlayerTrigger(uid, call.triggerPath)

            if opts.onTimeout then
                opts.onTimeout()
            end
        end)
    end
    return callID
end

function mondrop.deleteDropTrigger(uid, handle)
    assertType(uid, 'integer')
    assertType(handle, 'integer')

    local call = _RSVD_NAME_activeDropCalls[handle]
    if not call then
        return
    end
    assert(call.uid == uid)

    _RSVD_NAME_activeDropCalls[handle] = nil
    if call.timerKey then
        closeThread(call.timerKey)
    end
    _RSVD_NAME_removePlayerTrigger(uid, call.triggerPath)
end

return mondrop
