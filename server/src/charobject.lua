local _RSVD_NAME_triggers = {}
local _RSVD_NAME_triggerSeqID = 0
function addTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    assert(triggerType >= SYS_ON_BEGIN, triggerType)
    assert(triggerType <  SYS_ON_END  , triggerType)

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

    _RSVD_NAME_triggers[triggerType][triggerSeqID] = nil

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

    local doneKeyList = {}
    for _, triggerKey in ipairs(triggerKeyList) do
        -- the callback may already have been removed by a reentrant deleteTrigger triggered
        -- from an earlier callback in this same snapshot
        local triggerFunc = _RSVD_NAME_triggers[triggerType] and _RSVD_NAME_triggers[triggerType][triggerKey]
        if triggerFunc then
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
    end

    if _RSVD_NAME_triggers[triggerType] then
        for _, key in ipairs(doneKeyList) do
            _RSVD_NAME_triggers[triggerType][key] = nil
        end

        if tableEmpty(_RSVD_NAME_triggers[triggerType]) then
            _RSVD_NAME_triggers[triggerType] = nil
        end
    end
end
