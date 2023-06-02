--, u8R"###(
--

function getUIDQuestAutoSaveVars(uid)
    local _RSVD_NAME_oldvals = dbGetUIDQuestVars(uid) or {}
    local _RSVD_NAME_newvals = {}
    local _RSVD_NAME_deletes = {}

    return setmetatable({}, {
        __index = function(_, k)
            if __RSVD_NAME_deletes[k] then
                return nil
            end

            if _RSVD_NAME_newvals[k] ~= nil then
                return _RSVD_NAME_newvals[k]
            end

            return _RSVD_NAME_oldvals[k]
        end,

        __newindex = function(_, k, v)
            if v == nil then
                if _RSVD_NAME_oldvals[k] ~= nil then
                    _RSVD_NAME_deletes[k] = true
                end
                _RSVD_NAME_newvals[k] = nil
            else
                if _RSVD_NAME_oldvals[k] ~= v then
                    _RSVD_NAME_newvals[k] = v
                end
                _RSVD_NAME_deletes[k] = nil
            end
        end,

        __close = function()
            if tableEmpty(_RSVD_NAME_newvals) and tableEmpty(_RSVD_NAME_deletes) then
                return
            end

            for k, _ in pairs(_RSVD_NAME_deletes) do
                _RSVD_NAME_oldvals[k] = nil
            end

            for k, v in pairs(_RSVD_NAME_newvals) do
                _RSVD_NAME_oldvals[k] = v
            end

            if tableEmpty(_RSVD_NAME_oldvals) then
                dbSetUIDQuestVars(uid, nil)
            else
                dbSetUIDQuestVars(uid, _RSVD_NAME_oldvals)
            end
        end,

        -- __pairs = function()
        --     return next, _RSVD_NAME_oldvals, nil
        -- end,
        --
        -- __ipairs = function()
        --     local function iter(t, i)
        --         local j = i + 1
        --         local v = t[j]
        --         if v ~= nil then
        --             return j, v
        --         end
        --     end
        --     return iter, _RSVD_NAME_oldvals, 0
        -- end,
        --
        -- __len = function()
        --     return #tb
        -- end
    })
end

function dbGetUIDQuestState(uid)
    return (dbGetUIDQuestVars(uid) or {})[SYS_QUESTVAR_STATE]
end

function dbSetUIDQuestState(uid, state)
    local questVars<close> = getUIDQuestAutoSaveVars(uid)
    questVars[SYS_QUESTVAR_STATE] = state
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
