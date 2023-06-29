--, u8R"###(
--

function postString(msg, ...)
    postRawString(msg:format(...))
end

function randomMove()
    return _RSVD_NAME_callFuncCoop('randomMove')
end

function spaceMove(arg1, arg2, arg3)
    local mapID = nil
    local x     = nil
    local y     = nil

    if type(arg1) == 'table' then
        -- ignore arg2 and arg3
        -- arg1 is an array with format: {mapName|mapID, x, y}

        assert(isArray(arg1))
        assert(tableSize(arg1) >= 3)

        if type(arg1[1]) == 'string' then
            mapID = getMapID(arg1[1])

        elseif math.type(arg1[1]) == 'integer' and arg1[1] >= 0 then
            mapID = arg1[1]

        else
            fatalPrintf("Invalid map: %s", type(arg1[1]))
        end

        assertType(arg1[2], 'integer')
        assertType(arg1[3], 'integer')

        x = arg1[2]
        y = arg1[3]

    else
        assertType(arg1, 'integer', 'string')
        assertType(arg2, 'integer')
        assertType(arg3, 'integer')

        if type(arg1) == 'string' then
            mapID = getMapID(arg1)

        elseif arg1 >= 0 then
            mapID = arg1

        else
            fatalPrintf("Invalid map: %s", arg1)
        end

        x = arg2
        y = arg3
    end

    return _RSVD_NAME_callFuncCoop('spaceMove', mapID, x, y)
end

function getTeamMemberList()
    return _RSVD_NAME_callFuncCoop('getTeamMemberList')
end

function getQuestState(questName)
    assertType(questName, 'string')
    local questUID = _RSVD_NAME_callFuncCoop('queryQuestUID', questName)

    assertType(questUID, 'integer')
    if questUID ~= 0 then
        return uidExecute(questUID, [[ return dbGetUIDQuestState(%d) ]], getUID())
    end
end

function _RSVD_NAME_coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

local _RSVD_NAME_triggers = {}
local _RSVD_NAME_triggerSeqID = 0
function addTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    if not _RSVD_NAME_triggers[triggerType] then
        _RSVD_NAME_triggers[triggerType] = {}
    end

    _RSVD_NAME_triggerSeqID = _RSVD_NAME_triggerSeqID + 1
    _RSVD_NAME_triggers[triggerType][_RSVD_NAME_triggerSeqID] = callback
    return {triggerType, _RSVD_NAME_triggerSeqID}
end

function deleteTrigger(triggerPath)
    assert(isArray(triggerPath))
    local triggerType  = triggerPath[1]
    local triggerSeqID = triggerPath[2]

    if not _RSVD_NAME_triggers[triggerType] then
        return
    end

    _RSVD_NAME_triggers[triggerType][_RSVD_NAME_triggerSeqID] = nil

    if tableEmpty(_RSVD_NAME_triggers[triggerType]) then
        _RSVD_NAME_triggers[triggerType] = nil
    end
end

function _RSVD_NAME_trigger(triggerType, ...)
    assertType(triggerType, 'integer')
    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

    local args = table.pack(...)
    local config = _RSVD_NAME_triggerConfig(triggerType)

    if not config then
        fatalPrintf('Can not process trigger type %d', triggerType)
    end

    if args.n > #config[2] then
        addLog(LOGTYPE_WARNING, 'Trigger type %s expected %d parameters, got %d', config[1], #config[2], args.n)
    end

    for i = 1, #config[2] do
        if type(args[i]) ~= config[2][i] and math.type(args[i]) ~= config[2][i] then
            fatalPrintf('The %d-th parmeter of trigger type %s expected %s, got %s', i, config[1], config[2][i], type(args[i]))
        end
    end

    if config[3] then
        config[3](args)
    end

    local doneKeyList = {}
    for triggerKey, triggerFunc in pairs(_RSVD_NAME_triggers[triggerType]) do
        local result = triggerFunc(table.unpack(args, 1, #config[2]))
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
    end
end

for triggerType = SYS_ON_BEGIN, (SYS_ON_END - 1)
do
    addTrigger(triggerType, function(...)
        for _, questUID in ipairs(_RSVD_NAME_callFuncCoop('queryQuestTriggerList', triggerType)) do
            -- don't use uidExecute(questUID, ...) here
            -- player shall not assume the trigger callback function name in target quest

            -- post trigger parameter to quest
            -- won't wait for trigger done, don't assume target quest will reply
            _RSVD_NAME_runQuestTrigger(questUID, triggerType, ...)
        end
    end)
end

--
-- )###"
