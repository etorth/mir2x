--, u8R"###(
--

local _RSVD_NAME_autoSaveHelpers = {}
function getUIDQuestAutoSaveVars(uid)
    assertType(uid, 'integer')
    local autoSaveInstance = _RSVD_NAME_autoSaveHelpers[uid]
    if autoSaveInstance then
        autoSaveInstance.recursive = autoSaveInstance.recursive + 1
        return autoSaveInstance.metatable
    end

    local newInstance = {
        oldvars = dbGetUIDQuestVars(uid) or {},
        newvars = {},
        deletes = {},

        -- need to support recursive calls
        -- otherwise inside change can get overwritten by outside copy, as following
        --
        --     do
        --         local outside<close> = getUIDQuestAutoSaveVars(uid)
        --         outside.a = 12 -- change value only
        --
        --         do
        --              local inside<local> = getUIDQuestAutoSaveVars(uid)
        --              inside.b = nil -- erase key
        --         end
        --     end
        --
        -- internal scope erases key b, but outside has an independent copy
        -- then when leaving outside scope, a gets restored
        recursive = 0,
    }

    local metatable = setmetatable({}, {
        __index = function(_, k)
            if newInstance.deletes[k] then
                return nil
            end

            if newInstance.newvals[k] ~= nil then
                return newInstance.newvals[k]
            end

            return newInstance.oldvals[k]
        end,

        __newindex = function(_, k, v)
            if v == nil then
                if newInstance.oldvals[k] ~= nil then
                    newInstance.deletes[k] = true
                end
                newInstance.newvals[k] = nil
            else
                if newInstance.oldvals[k] ~= v then
                    newInstance.newvals[k] = v
                end
                newInstance.deletes[k] = nil
            end
        end,

        __close = function()
            if newInstance.recursive > 1 then
                newInstance.recursive = newInstance.recursive - 1
                return
            end

            if tableEmpty(newInstance.newvals) and tableEmpty(newInstance.deletes) then
                return
            end

            for k, _ in pairs(newInstance.deletes) do
                newInstance.oldvals[k] = nil
            end

            for k, v in pairs(newInstance.newvals) do
                newInstance.oldvals[k] = v
            end

            if tableEmpty(newInstance.oldvals) then
                dbSetUIDQuestVars(uid, nil)
            else
                dbSetUIDQuestVars(uid, newInstance.oldvals)
            end

            _RSVD_NAME_autoSaveHelpers[uid] = nil
        end,
    })

    newInstance.metatable = metatable
    _RSVD_NAME_autoSaveHelpers[uid] = newInstance
    return metatable
end

function dbUpdateUIDQuestVar(uid, key, value)
    assertType(uid, 'integer')
    local questVars<close> = getUIDQuestAutoSaveVars(uid)
    questVars[key] = value
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

    local npcUID = uidExecute(mapUID, [[
        return getNPCharUID('%s')
    ]], npcName)

    assertType(npcUID, 'integer')
    return npcUID
end

function getUIDQuestTeamLeader(uid)
    -- TODO
end

function getUIDQuestTeamMemberList(uid)
    assertType(uid, 'integer')
    local teamMemberList = nil
    do
        local questVars<close> = getUIDQuestAutoSaveVars(uid)
        if questVars[SYS_QUESTVAR_TEAMMEMBERLIST] then
            teamMemberList = questVars[SYS_QUESTVAR_TEAMMEMBERLIST]
        else
            teamMemberList = uidExecute(uid, [[ return getTeamMemberList() ]])
            questVars[SYS_QUESTVAR_TEAMMEMBERLIST] = teamMemberList

            for _, teamMember in ipairs(teamMemberList) do
                if teamMember ~= uid then
                    dbUpdateUIDQuestVar(teamMember, SYS_QUESTVAR_TEAMMEMBERLIST, teamMemberList)
                end
            end
        end
    end
    return teamMemberList
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

function setQuestState(uid, state)
    assertType(uid, 'integer')
    if (not hasQuestState(state)) and (state ~= SYS_EXIT) then
        fatalPrintf('Invalid quest state: %s', state)
    end

    -- don't save team member list here
    -- a player can be in a team but still start a single-role quest alone

    dbSetUIDQuestState(uid, state)
    if hasQuestState(state) then
        _RSVD_NAME_questFSMTable[state](uid)
    end
end

function dbRestoreQuestState(uid)
    setQuestState(uid, dbGetUIDQuestState(uid))
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
