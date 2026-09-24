-- run func on its own lua thread in this quest, returns the key to cancel it by
--
-- this is how a script gets a timer: pause() inside the thread and the rest runs later, and
-- since pause is cancellable, closeThread(key) cancels the timer with it
--
--     local timer = runQuestThread(function()
--         pause(3 * 60 * 1000)
--         giveUp(uid)
--     end)
--
--     closeThread(timer)      -- they finished in time
--
-- rollKey is unique per call, so the key alone identifies the thread and the seqID does not
-- have to be carried around. from inside the thread use getKeyPair()
function runQuestThread(func)
    assertType(func, 'function')

    local key = rollKey()
    runThread(key, func)
    return key
end

local function _RSVD_NAME_dbUpdateQuestFieldTable(uid, field, key, value)
    assertType(uid, 'integer')
    assertType(field, 'string')

    local fieldTable = dbGetQuestField(uid, field)
    assertType(fieldTable, 'nil', 'table')

    if fieldTable == nil then
        fieldTable = {}
    end

    if fieldTable[key] == value then
        return
    end

    fieldTable[key] = value

    if tableEmpty(fieldTable) then
        fieldTable = nil
    end
    dbSetQuestField(uid, field, fieldTable)
end

function dbGetQuestVar(uid, key)
    assertType(uid, 'integer')
    return (dbGetQuestField(uid, 'fld_vars') or {})[key]
end

function dbSetQuestVar(uid, key, value)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_vars', key, value)
end

function dbGetQuestState(uid, fsm)
    assertType(uid, 'integer')
    assertType(fsm, 'string', 'nil')

    local states = dbGetQuestField(uid, 'fld_states')
    if fsm == nil then
        fsm = SYS_QSTFSM
    end

    for k, v in pairs(states or {}) do
        if k == fsm then
            return v[1], v[2]
        end
    end
end

function _RSVD_NAME_dbGetQuestStateList(uid)
    assertType(uid, 'integer')
    return dbGetQuestField(uid, 'fld_states')
end

function hasQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    assertType(flagName, 'string')

    local flags = dbGetQuestField(uid, 'fld_flags') or {}
    return flags[flagName] or false
end

function addQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_flags', flagName, true)
end

function deleteQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_flags', flagName, nil)
end

function getNPCharUID(mapName, npcName)
    local mapUID = loadBaseMap(mapName)

    if not mapUID then
        return nil
    end

    local npcUID = uidRemoteCall(mapUID, npcName,
    [[
        local npcName = ...
        return getNPCharUID(npcName)
    ]])

    assertType(npcUID, 'integer', 'nil')
    return npcUID
end

function setQuestTeam(args)
    assertType(args, 'table')
    assertType(args.uid, 'integer')
    assertType(args.randRole, 'boolean', 'nil')
    assertType(args.propagate, 'boolean', 'nil')

    local team = uidRemoteCall(args.uid,
    [[
        return {
            [SYS_QUESTFIELD.TEAM.LEADER] = getTeamLeader(),
            [SYS_QUESTFIELD.TEAM.MEMBERLIST] = getTeamMemberList(),
        }
    ]])

    if args.randRole then
        team[SYS_QUESTFIELD.TEAM.ROLELIST] = shuffleArray(team[SYS_QUESTFIELD.TEAM.MEMBERLIST])
    else
        team[SYS_QUESTFIELD.TEAM.ROLELIST] = team[SYS_QUESTFIELD.TEAM.MEMBERLIST]
    end

    for _, member in ipairs(team[SYS_QUESTFIELD.TEAM.MEMBERLIST]) do
        if args.propagate or (member == uid) then
            dbSetQuestField(member, 'fld_team', team)
        end
    end
end

function getQuestTeam(uid)
    assertType(uid, 'integer')
    local team = dbGetQuestField(uid, 'fld_team')

    team.getRoleIndex = function(self, uid)
        assertType(self, 'table')
        assertType(uid, 'integer')

        for i, teamMember in ipairs(self[SYS_QUESTFIELD.TEAM.ROLELIST]) do
            if teamMember == uid then
                return i
            end
        end
        fatalPrintf('Can not find uid %d in team role list', uid)
    end
    return team
end

local _RSVD_NAME_questFSMTable = nil
function setQuestFSMTable(arg1, arg2)
    local fsmName  = nil
    local fsmTable = nil

    if type(arg1) == 'string' and type(arg2) == 'table' then
        fsmName  = arg1
        fsmTable = arg2

    elseif type(arg1) == 'table' and arg2 == nil then
        fsmName  = SYS_QSTFSM
        fsmTable = arg1

    else
        fatalPrintf('Invalid arguments')
    end

    assertType(fsmTable[SYS_ENTER], 'function')

    if getThreadKey() ~= getMainScriptThreadKey() then
        fatalPrintf('Can not modify quest FSM table other than in main script thread')
    end

    if _RSVD_NAME_questFSMTable == nil then
        _RSVD_NAME_questFSMTable = {}
    end
    _RSVD_NAME_questFSMTable[fsmName] = fsmTable
end

function hasQuestFSM(fsm)
    assertType(fsm, 'string')
    if not _RSVD_NAME_questFSMTable      then return false end
    if not _RSVD_NAME_questFSMTable[fsm] then return false end
    return true
end

function hasQuestState(arg1, arg2)
    assertType(arg1, 'string')
    assertType(arg2, 'string', 'nil')

    local fsmName = nil
    local state   = nil

    if arg2 == nil then
        fsmName = SYS_QSTFSM
        state   = arg1
    else
        fsmName = arg1
        state   = arg2
    end

    if not _RSVD_NAME_questFSMTable                 then return false end
    if not _RSVD_NAME_questFSMTable[fsmName]        then return false end
    if not _RSVD_NAME_questFSMTable[fsmName][state] then return false end
    return true
end

function setQuestState(fargs)
    assertType(fargs, 'table')
    assertType(fargs.uid, 'integer')
    assertType(fargs.fsm, 'string', 'nil')
    assertType(fargs.state, 'string')
    assertType(fargs.exitfunc, 'function', 'string', 'nil')

    if type(fargs.exitfunc) == 'string' then
        assertType(fargs.exitargs, 'string', 'table', 'nil')
    else
        assertType(fargs.exitargs, 'nil')
    end

    local uid   = fargs.uid
    local fsm   = fargs.fsm or SYS_QSTFSM
    local state = fargs.state

    if (not hasQuestState(fsm, state)) and (state ~= SYS_DONE) then
        fatalPrintf('Invalid arguments: fsm %s, state %s', fsm, state)
    end

    -- don't save team member list here
    -- a player can be in a team but still start a single-role quest alone

    if (fsm == SYS_QSTFSM) and (state == SYS_DONE) then
        local npcBehaviors = dbGetQuestField(uid, 'fld_npcbehaviors')
        if npcBehaviors then
            for _, v in pairs(npcBehaviors) do
                clearNPCQuestBehavior(v[1], v[2], uid)
            end
        end

        local gridTriggers = dbGetQuestField(uid, 'fld_gridtriggers')
        if gridTriggers then
            local mapNameList = {}
            for _, v in pairs(gridTriggers) do
                mapNameList[v[1]] = true
            end

            for mapName in pairs(mapNameList) do
                _RSVD_NAME_clearQuestMapUIDGridTrigger(mapName, uid)
            end
        end
        dbSetQuestField(uid, 'fld_gridtriggers', nil)
        _RSVD_NAME_dbSetQuestStateDone(uid)
    else
        if (state ~= SYS_DONE) and (not dbGetQuestState(uid, fsm)) then
            setQuestDesp{uid=uid, fsm=fsm, ''}
        end
        _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_states', fsm, {state, fargs.args})
    end

    -- if not called from another FSM state op
    -- fsm name is nil

    local currFSMName = _RSVD_NAME_currFSMName
    assertType(currFSMName, 'string', 'nil')

    _RSVD_NAME_closeQuestState(uid, fsm)
    if hasQuestState(fsm, state) then
        runQuestThread(function()
            _RSVD_NAME_enterQuestState(uid, fsm, state, fargs.args)
            if type(fargs.exitfunc) == 'function' then
                runQuestThread(fargs.exitfunc)
            elseif type(fargs.exitfunc) == 'string' then
                local exitfunc = load(fargs.exitfunc)
                local exitargs = (function()
                    if type(fargs.exitargs) == 'table' then
                        return fargs.exitargs
                    elseif type(fargs.exitargs) == 'string' then
                        return table.pack(load(fargs.exitargs)())
                    elseif type(fargs.exitargs) == 'nil' then
                        return table.pack()
                    else
                        fatalPrintf('Invalid exitargs type: %s', type(fargs.exitargs))
                    end
                end)()
                runQuestThread(function()
                    exitfunc(table.unpack(exitargs, 1, exitargs.n))
                end)
            elseif fargs.exitfunc ~= nil then
                fatalPrintf('Invalid exitfunc type: %s', type(fargs.exitfunc))
            end
        end)
    end

    -- drop current thread in C layer
    -- next state will be executed in a new thread

    if currFSMName == fsm then
        while true do
            coroutine.yield()
        end
    end
end

function dbGetQuestDesp(uid)
    assertType(uid, 'integer')
    return dbGetQuestField(uid, 'fld_desp')
end

function setQuestDesp(args)
    assertType(args, 'table')
    assertType(args.uid, 'integer')
    assertType(args.fsm, 'string', 'nil')

    local uid = args.uid
    local fsm = args.fsm or SYS_QSTFSM

    assert(hasQuestFSM(fsm), string.format('Invalid fsm name: %s', fsm))

    local desp = (function()
        if args.format == nil and #args == 0 then
            return nil

        elseif args.format ~= nil then
            assertType(args.format, 'string')
            return string.format(args.format, table.unpack(args, 1, #args))

        else
            assertType(args[1], 'string')
            return string.format(table.unpack(args, 1, #args))
        end
    end)()

    local newDespTable = (function()
        if fsm == SYS_QSTFSM and desp == nil then
            return nil
        end

        local despTable = dbGetQuestField(uid, 'fld_desp') or {}
        despTable[fsm] = desp

        if tableEmpty(despTable) then
            return nil
        else
            return despTable
        end
    end)()

    _RSVD_NAME_setQuestDesp(uid, newDespTable, fsm, desp)
end

-- setup NPC chat logics
-- also save to database for next time loading, usage:
--
--     setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
--     [[
--         return getUID(), getQuestName()
--     ]],
--
--     [[
--         local questUID, questName = ...
--         local questPath = {SYS_EPUID, questName}
--
--         return
--         {
--             [SYS_ENTER] = function(uid, value)
--                 uidPostXML(uid, questPath,
--                 [=[
--                     <layout>
--                         <par>是士官派你来的？</par>
--                         <par>嗯，那么先吩咐你做件简单的事儿吧！你能去把这个护身符交给武器库的<t color="red">阿潘</t>道友吗？</par>
--                         <par></par>
--                         <par><event id="npc_accept_quest">好的！</event></par>
--                     </layout>
--                 ]=])
--             end,
--
-- argstr shouldn't capture current environ's values
-- argstr get evalulated everytime when when setup the NPC behavior
--
function setupNPCQuestBehavior(mapName, npcName, uid, arg1, arg2)
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    assertType(uid, 'integer')
    assert(uid > 0)

    local argstr = nil
    local code   = nil

    if type(arg1) == 'string' and type(arg2) == 'string' then
        argstr = arg1
        code   = arg2

    elseif type(arg1) == 'string' and arg2 == nil then
        argstr = nil
        code   = arg1

    elseif arg1 == nil and type(arg2) == 'string' then
        argstr = nil
        code   = arg2

    else
        fatalPrintf('Invalid arguments to setupNPCQuestBehavior(%s, %s, %d, ...)', asInitString(mapName), asInitString(npcName), uid)
    end

    -- re-evalulate argstr to capture current environ's values
    -- don't save the environ's specific value to database, which may causes error for next time loading

    local args = argstr and table.pack(load(argstr)()) or table.pack()
    args[args.n + 1] =
    [[
        local playerUID, questName, code = ...
        setUIDQuestHandler(playerUID, questName, load(code)(select(4, ...)))
    ]]

    -- use array as {code, argstr}
    -- argstr can be nil, put ahead may cause trouble

    uidRemoteCall(getNPCharUID(mapName, npcName), uid, getQuestName(), code, table.unpack(args, 1, args.n + 1))
    _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_npcbehaviors', strAny({mapName, npcName}), {mapName, npcName, code, argstr})
end

-- setupNPCQuestBehavior against one map copy instead of a map name
--
-- deliberately not persisted: a copy does not survive a restart, so on the next login there
-- is no map to reinstall onto and the quest state alone decides what happens
function setupInstanceNPCBehavior(mapUID, npcName, uid, arg1, arg2)
    assertType(mapUID, 'integer')
    assertType(npcName, 'string')

    assertType(uid, 'integer')
    assert(uid > 0)

    local argstr = nil
    local code   = nil

    if type(arg1) == 'string' and type(arg2) == 'string' then
        argstr = arg1
        code   = arg2

    elseif type(arg1) == 'string' and arg2 == nil then
        argstr = nil
        code   = arg1

    elseif arg1 == nil and type(arg2) == 'string' then
        argstr = nil
        code   = arg2

    else
        fatalPrintf('Invalid arguments to setupInstanceNPCBehavior(%d, %s, %d, ...)', mapUID, asInitString(npcName), uid)
    end

    local npcUID = uidRemoteCall(mapUID, npcName,
    [[
        local npcName = ...
        return getNPCharUID(npcName)
    ]])

    assertType(npcUID, 'integer', 'nil')
    if not npcUID then
        fatalPrintf('No NPC %s on map copy %d', asInitString(npcName), mapUID)
    end

    local args = argstr and table.pack(load(argstr)()) or table.pack()
    args[args.n + 1] =
    [[
        local playerUID, questName, code = ...
        setUIDQuestHandler(playerUID, questName, load(code)(select(4, ...)))
    ]]

    uidRemoteCall(npcUID, uid, getQuestName(), code, table.unpack(args, 1, args.n + 1))
end

function clearNPCQuestBehavior(mapName, npcName, uid)
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    assertType(uid, 'integer')
    assert(uid > 0)

    uidRemoteCall(getNPCharUID(mapName, npcName), uid, getQuestName(), [[ deleteUIDQuestHandler(...) ]])
    _RSVD_NAME_dbUpdateQuestFieldTable(uid, 'fld_npcbehaviors', strAny({mapName, npcName}), nil)
end

local function parseUIDGridTriggerArgs(funcName, ...)
    local args = table.pack(...)
    local rectList = nil
    local x        = nil
    local y        = nil
    local uid      = nil
    local argstr   = nil
    local code     = nil

    if type(args[1]) == 'table' and args.n == 3 then
        rectList, uid, code = table.unpack(args, 1, 3)

    elseif type(args[1]) == 'table' and args.n == 4 then
        rectList, uid, argstr, code = table.unpack(args, 1, 4)

    elseif math.type(args[1]) == 'integer' and args.n == 4 then
        x, y, uid, code = table.unpack(args, 1, 4)

    elseif math.type(args[1]) == 'integer' and args.n == 5 then
        x, y, uid, argstr, code = table.unpack(args, 1, 5)

    else
        fatalPrintf('Invalid arguments to %s()', funcName)
    end

    if rectList then
        assertType(rectList, 'table')
    else
        assertType(x, 'integer')
        assertType(y, 'integer')
    end
    assertType(uid, 'integer')
    assert(uid > 0)
    assertType(argstr, 'string', 'nil')
    assertType(code, 'string')

    return
    {
        rectList = rectList,
        x        = x,
        y        = y,
        uid      = uid,
        argstr   = argstr,
        code     = code,
    }
end

local function parseGridTriggerArgs(funcName, ...)
    local args = table.pack(...)
    local rectList = nil
    local x        = nil
    local y        = nil
    local argstr   = nil
    local code     = nil

    if type(args[1]) == 'table' and args.n == 2 then
        rectList, code = table.unpack(args, 1, 2)

    elseif type(args[1]) == 'table' and args.n == 3 then
        rectList, argstr, code = table.unpack(args, 1, 3)

    elseif math.type(args[1]) == 'integer' and args.n == 3 then
        x, y, code = table.unpack(args, 1, 3)

    elseif math.type(args[1]) == 'integer' and args.n == 4 then
        x, y, argstr, code = table.unpack(args, 1, 4)

    else
        fatalPrintf('Invalid arguments to %s()', funcName)
    end

    if rectList then
        assertType(rectList, 'table')
    else
        assertType(x, 'integer')
        assertType(y, 'integer')
    end
    assertType(argstr, 'string', 'nil')
    assertType(code, 'string')

    return
    {
        rectList = rectList,
        x        = x,
        y        = y,
        argstr   = argstr,
        code     = code,
    }
end

-- take over one grid or a rect list of a map for one player
--
-- the grid stops sending the player through on its own, the installed code decides, return
-- true from it to let the player continue to wherever the grid leads
--
-- rect lists use {{x, y, w, h}, ...}; the whole list is installed as one trigger
--
--     setupMapUIDGridTrigger('半兽洞穴2层_D002', 225, 175, uid,
--     [[
--         return getQuestName()
--     ]],
--     [[
--         local questName = ...
--         return function(uid, x, y)
--             if server.player.hasItem(uid, '不死牌', 1) then
--                 return true
--             end
--             server.player.postString(uid, '不知道为什么，门被反锁了，无法进入……')
--             return false
--         end
--     ]])
--
-- like setupNPCQuestBehavior the argstr is re-evaluated on every install, so it must not
-- capture anything from the current environment
function setupMapUIDGridTrigger(mapName, ...)
    assertType(mapName, 'string')

    local config = parseUIDGridTriggerArgs('setupMapUIDGridTrigger', ...)
    local rectList = config.rectList or {{config.x, config.y, 1, 1}}

    local mapUID = loadBaseMap(mapName)
    if not mapUID then
        fatalPrintf('Can not load map %s', asInitString(mapName))
    end

    local args = config.argstr and table.pack(load(config.argstr)()) or table.pack()
    args[args.n + 1] =
    [[
        local playerUID, questName, rectList, code = ...
        addUIDGridTrigger(playerUID, questName, rectList, load(code)(select(5, ...)))
    ]]

    local triggerId = assertType(uidRemoteCall(mapUID, config.uid, getQuestName(), rectList, config.code, table.unpack(args, 1, args.n + 1)), 'integer')
    local storageKey = nil
    local storageValue = nil

    if config.rectList then
        storageKey = strAny({mapName, config.rectList})
        storageValue = {mapName, config.rectList, config.code, config.argstr}
    else
        storageKey = strAny({mapName, config.x, config.y})
        storageValue = {mapName, config.x, config.y, config.code, config.argstr}
    end

    _RSVD_NAME_dbUpdateQuestFieldTable(config.uid, 'fld_gridtriggers', storageKey, storageValue)
    return triggerId
end

-- setupMapUIDGridTrigger against one map copy instead of a map name, also accepts a rect list
--
-- this is how two instance copies get linked to each other: the gate grid on a copy still
-- carries the mapSwitchList destination, which only ever names the base map, so a quest that
-- loaded copies of both maps installs a trigger here, returns false to refuse the automatic
-- switch, and spaceMoves the player into its own copy of the far side by uid
--
-- deliberately not persisted, for the same reason as setupInstanceNPCBehavior: a copy does not
-- survive a restart and there is nothing to reinstall onto
function setupInstanceUIDGridTrigger(mapUID, ...)
    assertType(mapUID, 'integer')

    local config = parseUIDGridTriggerArgs('setupInstanceUIDGridTrigger', ...)
    local rectList = config.rectList or {{config.x, config.y, 1, 1}}

    local args = config.argstr and table.pack(load(config.argstr)()) or table.pack()
    args[args.n + 1] =
    [[
        local playerUID, questName, rectList, code = ...
        addUIDGridTrigger(playerUID, questName, rectList, load(code)(select(5, ...)))
    ]]

    return assertType(uidRemoteCall(mapUID, config.uid, getQuestName(), rectList, config.code, table.unpack(args, 1, args.n + 1)), 'integer')
end

-- take one grid or a rect list of a map over for everyone on it, not just one player
--
-- this is the SYS_EPDEF half of the grid trigger layer, and it is what gates a door against
-- players who are not on the quest at all. a per-player trigger from setupMapUIDGridTrigger is
-- consulted first, so the two compose: the quest installs EPUID to let its own player through
-- and EPDEF to turn everybody else away
--
--     setupMapGridTrigger('沃玛神殿2层_D023', 371, 366,
--     [[
--         return getUID()
--     ]],
--     [[
--         local questUID = ...
--         return function(uid, x, y)
--             server.player.postString(uid, '(现在好像进不去了...)')
--             return false
--         end
--     ]])
--
-- deliberately not persisted: nothing about it is per-player, and a quest script re-runs from
-- the top on every server start, which is where this belongs
function setupMapGridTrigger(mapName, ...)
    assertType(mapName, 'string')

    local config = parseGridTriggerArgs('setupMapGridTrigger', ...)
    local rectList = config.rectList or {{config.x, config.y, 1, 1}}

    local mapUID = loadBaseMap(mapName)
    if not mapUID then
        fatalPrintf('Can not load map %s', asInitString(mapName))
    end

    local args = config.argstr and table.pack(load(config.argstr)()) or table.pack()
    args[args.n + 1] =
    [[
        local rectList, code = ...
        addGridTrigger(rectList, load(code)(select(3, ...)))
    ]]

    return assertType(uidRemoteCall(mapUID, rectList, config.code, table.unpack(args, 1, args.n + 1)), 'integer')
end

function clearMapGridTrigger(mapName, triggerId)
    assertType(mapName, 'string')
    assertType(triggerId, 'integer')

    local mapUID = loadBaseMap(mapName)
    if mapUID then
        uidRemoteCall(mapUID, triggerId, [[ deleteGridTrigger(...) ]])
    end
end

function clearMapUIDGridTrigger(mapName, triggerId)
    assertType(mapName, 'string')
    assertType(triggerId, 'integer')

    local mapUID = loadBaseMap(mapName)
    if mapUID then
        uidRemoteCall(mapUID, triggerId, [[ deleteUIDGridTrigger(...) ]])
    end
end

-- remove every SYS_EPUID grid trigger this player has for the current quest
-- used by setQuestState() when the quest reaches SYS_DONE
function _RSVD_NAME_clearQuestMapUIDGridTrigger(mapName, uid)
    assertType(mapName, 'string')
    assertType(uid, 'integer')

    local mapUID = loadBaseMap(mapName)
    if mapUID then
        uidRemoteCall(mapUID, uid, getQuestName(), [[ _RSVD_NAME_clearQuestUIDGridTrigger(...) ]])
    end
end

function runNPCEventHandler(npcUID, playerUID, eventPath, event, value)
    uidRemoteCall(npcUID, playerUID, eventPath, event, value,
    [[
        local playerUID, eventPath, event, value = ...
        runEventHandler(playerUID, eventPath, event, value)
    ]])
end

function _RSVD_NAME_enterQuestState(uid, fsm, state, args)
    assertType(uid, 'integer')
    assertType(fsm, 'string')
    assertType(state, 'string')

    if not hasQuestState(fsm, state) then
        fatalPrintf('Invalid quest: fsm %s, state %s', fsm, state)
    end

    _RSVD_NAME_currFSMName = fsm
    _RSVD_NAME_questFSMTable[fsm][state](uid, args)
end

local _RSVD_NAME_triggers = {}
local _RSVD_NAME_triggerSeqID = 0
function addQuestTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

    if not _RSVD_NAME_triggers[triggerType] then
        _RSVD_NAME_triggers[triggerType] = {}
        _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', triggerType, true)
    end

    _RSVD_NAME_triggerSeqID = _RSVD_NAME_triggerSeqID + 1
    _RSVD_NAME_triggers[triggerType][_RSVD_NAME_triggerSeqID] = callback
    return {triggerType, _RSVD_NAME_triggerSeqID}
end

-- delte quest trigger
-- this disables all player to trigger the deleted one
function deleteQuestTrigger(triggerPath)
    assert(isArray(triggerPath))

    local triggerType  = triggerPath[1]
    local triggerSeqID = triggerPath[2]

    if not _RSVD_NAME_triggers[triggerType] then
        return
    end

    _RSVD_NAME_triggers[triggerType][triggerSeqID] = nil

    if tableEmpty(_RSVD_NAME_triggers[triggerType]) then
        _RSVD_NAME_triggers[triggerType] = nil
        _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', triggerType, false)
    end
end

-- whenever a player event happens it runs all locally installed triggers by addTrigger(). also
-- it sends event{type, args} to servicecore, servicecore dispatches it to all quests that sensitive to this event
-- which eventually call this function: _RSVD_NAME_trigger()
function _RSVD_NAME_trigger(triggerType, uid, ...)
    assertType(triggerType, 'integer')
    assertType(uid        , 'integer')

    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

    local args = table.pack(...)
    local config = _RSVD_NAME_triggerConfig(triggerType)

    if not config then
        fatalPrintf('Can not process trigger type %d', triggerType)
    end

    if args.n > #config[2] then
        addLog(LOGTYPE_WARNING, 'Trigger type %s expected %d parameters, got %d', config[1], #config[2] + 1, args.n + 1)
    end

    for i = 1, #config[2] do
        if type(args[i]) ~= config[2][i] and math.type(args[i]) ~= config[2][i] then
            fatalPrintf('The %d-th parmeter of trigger type %s expected %s, got %s', i + 1, config[1], config[2][i], type(args[i]))
        end
    end

    if config[3] then
        config[3](args)
    end

    -- snapshot (key, callback) pairs to prevent table mutation during pairs() iteration
    -- callbacks may yield (e.g. uidRemoteCall) or trigger reentrant addTrigger/deleteTrigger calls
    -- mutating _RSVD_NAME_triggers while pairs() runs corrupts the iterator and get error like 'invalid key to next'
    -- collecting pairs into a array first allows safe callback invocation afterward

    local triggerKeyList = {}
    if _RSVD_NAME_triggers[triggerType] then
        for triggerKey in pairs(_RSVD_NAME_triggers[triggerType]) do
            table.insert(triggerKeyList, triggerKey)
        end
    end

    for _, triggerKey in ipairs(triggerKeyList) do
        -- the callback may already have been removed by a reentrant deleteQuestTrigger
        -- triggered from an earlier callback in this same snapshot
        local triggerFunc = _RSVD_NAME_triggers[triggerType] and _RSVD_NAME_triggers[triggerType][triggerKey]
        if triggerFunc and triggerFunc(uid, table.unpack(args, 1, #config[2])) ~= nil then
            addLog(LOGTYPE_WARNING, 'Quest trigger %s callback shall never return any value other than nil')
        end
    end
end
