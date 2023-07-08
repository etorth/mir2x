--, u8R"###(
--

local function _RSVD_NAME_dbUpdateUIDQuestFieldTable(uid, field, key, value)
    assertType(uid, 'integer')
    assertType(field, 'string')

    local fieldTable = dbGetUIDQuestField(uid, field)
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
    dbSetUIDQuestField(uid, field, fieldTable)
end

function dbGetUIDQuestVar(uid, key)
    assertType(uid, 'integer')
    return (dbGetUIDQuestField(uid, 'fld_vars') or {})[key]
end

function dbSetUIDQuestVar(uid, key, value)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateUIDQuestFieldTable(uid, 'fld_vars', key, value)
end

function dbGetUIDQuestState(uid)
    assertType(uid, 'integer')
    return table.unpack(dbGetUIDQuestField(uid, 'fld_state') or {}, 1, 2)
end

function dbSetUIDQuestState(uid, state, args)
    assertType(uid, 'integer')
    assertType(state, 'string')
    dbSetUIDQuestField(uid, 'fld_state', {state, args})
end

function hasUIDQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    assertType(flagName, 'string')

    local flags = dbGetUIDQuestFlag(uid, 'fld_flags') or {}
    return flags[flagName] or false
end

function addUIDQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateUIDQuestFieldTable(uid, 'fld_flags', flagName, true)
end

function deleteUIDQuestFlag(uid, flagName)
    assertType(uid, 'integer')
    _RSVD_NAME_dbUpdateUIDQuestFieldTable(uid, 'fld_flags', flagName, nil)
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

function setUIDQuestTeam(args)
    assertType(args, 'table')
    assertType(args.uid, 'integer')
    assertType(args.randRole, 'boolean', 'nil')
    assertType(args.propagate, 'boolean', 'nil')

    local team = uidExecute(args.uid,
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
            dbSetUIDQuestField(member, 'fld_team', team)
        end
    end
end

function getUIDQuestTeam(uid)
    assertType(uid, 'integer')
    local team = dbGetUIDQuestField(uid, 'fld_team')

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

function setUIDQuestState(uid, state, args)
    assertType(uid, 'integer')
    if (not hasQuestState(state)) and (state ~= SYS_EXIT) then
        fatalPrintf('Invalid quest state: %s', state)
    end

    -- don't save team member list here
    -- a player can be in a team but still start a single-role quest alone

    dbSetUIDQuestState(uid, state, args)

    if hasQuestState(state) then
        _RSVD_NAME_switchUIDQuestState(uid, state, args, getTLSTable().threadKey, getTLSTable().threadSeqID)
    else
        _RSVD_NAME_switchUIDQuestState(uid,   nil,  nil, getTLSTable().threadKey, getTLSTable().threadSeqID)
    end

    -- drop current thread in C layer
    -- next state will be executed in a new thread

    while true do
        coroutine.yield()
    end
end

function setupNPCQuestBehavior(mapName, npcName, uid, argstr, code)
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    assertType(uid, 'integer')
    assert(uid > 0)

    assertType(argstr, 'string', 'nil')
    assertType(code, 'string')

    local args = table.pack(load(argstr)())
    args[args.n + 1] = string.format([[ setUIDQuestHandler(%d, %s, load(%s)(...)) ]], uid, asInitString(getQuestName()), asInitString(code))

    uidRemoteCall(getNPCharUID(mapName, npcName), table.unpack(args, 1, args.n + 1))
end

function clearNPCQuestBehavior(mapName, npcName, uid)
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    assertType(uid, 'integer')
    assert(uid > 0)

    uidRemoteCall(getNPCharUID(mapName, npcName), uid, getQuestName(), [[ deleteUIDQuestHandler(...) ]])
end

function _RSVD_NAME_enterUIDQuestState(uid, state, base64Args)
    assertType(uid, 'integer')
    assertType(state, 'string')
    assertType(base64Args, 'string', 'nil')

    if not hasQuestState(state) then
        fatalPrintf('Invalid quest state: %s', state)
    end

    if base64Args then
        _RSVD_NAME_questFSMTable[state](uid, base64Decode(base64Args))
    else
        _RSVD_NAME_questFSMTable[state](uid, nil)
    end
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
