--, u8R"###(
--

function postString(msg, ...)
    postRawString(msg:format(...))
end

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    local oldTime = getTime()
    _RSVD_NAME_pauseYielding(ms, getTLSTable().threadKey)
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
    if triggerType == SYS_ON_LEVELUP then
        assertType(args[1], 'integer')
        assertType(args[2], 'integer')
        assertType(args[3], 'nil')

        assert(args[1] < args[2])
        if _RSVD_NAME_triggers[SYS_ON_LEVELUP] then
            local doneKeyList = {}
            for triggerKey, triggerFunc in pairs(_RSVD_NAME_triggers[SYS_ON_LEVELUP]) do
                local result = triggerFunc(uid, args[1], args[2])
                if type(result) == 'boolean' then
                    if result then
                        table.insert(doneKeyList, triggerKey)
                    end
                elseif type(result) ~= 'nil' then
                    table.insert(doneKeyList, triggerKey)
                    addLog(LOGTYPE_WARNING, 'Trigger %s returns invalid type %s, trigger removed.', tostring(triggerKey), type(result))
                end
            end

            for _, key in ipairs(doneKeyList) do
                _RSVD_NAME_triggers[SYS_ON_LEVELUP][key] = nil
            end
        end

        if tableEmpty(_RSVD_NAME_triggers[SYS_ON_LEVELUP]) then
            _RSVD_NAME_triggers[SYS_ON_LEVELUP] = nil
        end
    elseif triggerType == SYS_ON_KILL then
    else
        fatalPrintf('Invalid trigger type: %d', triggerType)
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
