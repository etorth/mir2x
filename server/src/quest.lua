--, u8R"###(
--

-- TODO
-- move pause() function to ServerLuaCoroutineRunner
function pause(ms)
    if ms == SYS_POSINF then
        while true do
            coroutine.yield()
        end
    end

    assertType(ms, 'integer')
    assert(ms >= 0)

    local oldTime = getTime()
    _RSVD_NAME_pauseYielding(ms, getTLSTable().threadKey, getTLSTable().threadSeqID)
    return getTime() - oldTime
end

function dbUpdateUIDQuestVar(uid, key, value)
    assertType(uid, 'integer')
    local vars = dbGetUIDQuestVars(uid) or {}
    vars[key] = value

    if tableEmpty(vars) then
        dbSetUIDQuestVars(uid, nil)
    else
        dbSetUIDQuestVars(uid, vars)
    end
end

function dbGetUIDQuestState(uid)
    assertType(uid, 'integer')
    return (dbGetUIDQuestVars(uid) or {})[SYS_QUESTVAR_STATE]
end

function dbSetUIDQuestState(uid, state)
    assertType(uid, 'integer')
    dbUpdateUIDQuestVar(uid, SYS_QUESTVAR_STATE, state)
end

function loadMap(map)
    return _RSVD_NAME_callFuncCoop('loadMap', map)
end

function getNPCharUID(mapName, npcName)
    local mapUID = loadMap(mapName)
    assertType(mapUID, 'integer')

    if mapUID == 0 then
        return 0
    end

    local npcUID = uidExecute(mapUID,
    [[
        return getNPCharUID('%s')
    ]], npcName)

    assertType(npcUID, 'integer')
    return npcUID
end

function _RSVD_NAME_getUIDQuestTeam(uid)
    assertType(uid, 'integer')
    local vars = dbGetUIDQuestVars(uid) or {}
    if vars[SYS_QUESTVAR_TEAM] then
        return vars[SYS_QUESTVAR_TEAM]
    else
        fatalPrintf('Call setUIDQuestTeam(...) first')
    end
end

function setUIDQuestTeam(args)
    assertType(args, 'table')
    assertType(args.uid, 'integer')
    assertType(args.randRole, 'boolean', 'nil')
    assertType(args.propagate, 'boolean', 'nil')

    local team = uidExecute(args.uid,
    [[
        return {
            [SYS_QUESTVAR_TEAMLEADER] = getTeamLeader(),
            [SYS_QUESTVAR_TEAMMEMBERLIST] = getTeamMemberList(),
        }
    ]])

    if args.randRole then
        team[SYS_QUESTVAR_TEAMROLELIST] = shuffleArray(team[SYS_QUESTVAR_TEAMMEMBERLIST])
    else
        team[SYS_QUESTVAR_TEAMROLELIST] = team[SYS_QUESTVAR_TEAMMEMBERLIST]
    end

    for _, teamMember in ipairs(team[SYS_QUESTVAR_TEAMMEMBERLIST]) do
        if args.propagate or (teamMember == uid) then
            dbUpdateUIDQuestVar(teamMember, SYS_QUESTVAR_TEAM, team)
        end
    end
end

function getUIDQuestTeamLeader(uid)
    assertType(uid, 'integer')
    return _RSVD_NAME_getUIDQuestTeam(uid)[SYS_QUESTVAR_TEAMLEADER]
end

function getUIDQuestTeamMemberList(uid)
    assertType(uid, 'integer')
    return _RSVD_NAME_getUIDQuestTeam(uid)[SYS_QUESTVAR_TEAMMEMBERLIST]
end

function getUIDQuestTeamRoleList(uid)
    assertType(uid, 'integer')
    return _RSVD_NAME_getUIDQuestTeam(uid)[SYS_QUESTVAR_TEAMROLELIST]
end

function getUIDQuestTeamRoleIndex(uid)
    assertType(uid, 'integer')
    for i, teamMember in ipairs(getUIDQuestTeamRoleList(uid)) do
        if teamMember == uid then
            return i
        end
    end
    fatalPrintf('Can not find uid %d in team role list', uid)
end

local _RSVD_NAME_questFSMTable = nil
function setQuestFSMTable(fsm)
    assertType(fsm, 'table')
    assertType(fsm[SYS_ENTER], 'function')
    if getTLSTable().threadKey ~= getMainScriptThreadKey() then
        fatalPrintf('Can not modify quest FSM table other than in main script thread')
    end
    _RSVD_NAME_questFSMTable = fsm
end

function hasQuestState(state)
    if not _RSVD_NAME_questFSMTable        then return false end
    if not _RSVD_NAME_questFSMTable[state] then return false end
    return true
end

function setUIDQuestState(uid, state)
    assertType(uid, 'integer')
    if (not hasQuestState(state)) and (state ~= SYS_EXIT) then
        fatalPrintf('Invalid quest state: %s', state)
    end

    -- don't save team member list here
    -- a player can be in a team but still start a single-role quest alone

    dbSetUIDQuestState(uid, state)

    if hasQuestState(state) then
        _RSVD_NAME_switchUIDQuestState(uid, state, getTLSTable().threadKey, getTLSTable().threadSeqID)
    else
        _RSVD_NAME_switchUIDQuestState(uid,   nil, getTLSTable().threadKey, getTLSTable().threadSeqID)
    end

    -- drop current thread in C layer
    -- next state will be executed in a new thread

    while true do
        coroutine.yield()
    end
end

function _RSVD_NAME_enterUIDQuestState(uid, state)
    assertType(uid, 'integer')
    assertType(state, 'string')

    if not hasQuestState(state) then
        fatalPrintf('Invalid quest state: %s', state)
    end
    _RSVD_NAME_questFSMTable[state](uid)
end

function restoreUIDQuestState(uid)
    setUIDQuestState(uid, dbGetUIDQuestState(uid))
end

local _RSVD_NAME_triggers = {}
function addQuestTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

    if not _RSVD_NAME_triggers[triggerType] then
        _RSVD_NAME_triggers[triggerType] = {}
        _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', triggerType, true)
    end

    table.insert(_RSVD_NAME_triggers[triggerType], callback)
end

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

    local doneKeyList = {}
    for triggerKey, triggerFunc in pairs(_RSVD_NAME_triggers[triggerType]) do
        local result = triggerFunc(uid, table.unpack(args, 1, #config[2]))
        if type(result) == 'boolean' then
            if result then
                table.insert(doneKeyList, triggerKey)
            end
        elseif type(result) ~= 'nil' then
            table.insert(doneKeyList, triggerKey)
            addLog(LOGTYPE_WARNING, 'Trigger %s callback returns invalid type %s, key %s removed.', config[1], type(result), tostring(triggerKey))
        end
    end

    for _, key in ipairs(doneKeyList) do
        _RSVD_NAME_triggers[triggerType][key] = nil
    end

    if tableEmpty(_RSVD_NAME_triggers[triggerType]) then
        _RSVD_NAME_triggers[triggerType] = nil
        _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', triggerType, false)
    end
end

--
-- )###"
