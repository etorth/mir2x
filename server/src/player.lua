--, u8R"###(
--

function postString(msg, ...)
    postRawString(msg:format(...))
end

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    local oldTime = getTime()
    _RSVD_NAME_pauseYielding(ms, getTLSTable().threadKey, getTLSTable().threadSeqID)
    return getTime() - oldTime
end

function randomMove()
    return _RSVD_NAME_callFuncCoop('randomMove')
end

function spaceMove(mapID, x, y)
    return _RSVD_NAME_callFuncCoop('spaceMove', mapID, x, y)
end

function _RSVD_NAME_coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

local _RSVD_NAME_triggers = {}
function addTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    if not _RSVD_NAME_triggers[triggerType] then
        _RSVD_NAME_triggers[triggerType] = {}
    end
    table.insert(_RSVD_NAME_triggers[triggerType], callback)
end

function _RSVD_NAME_trigger(triggerType, ...)
    assertType(triggerType, 'integer')
    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

    local args = {...}
    local config = _RSVD_NAME_triggerConfig(triggerType)

    if not config then
        fatalPrintf('Can not process trigger type %d', triggerType)
    end

    if #args > #config[2] then
        addLog(LOGTYPE_WARNING, 'Trigger type %s expected %d parameters, got %d', config[1], #config[2], #args)
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
