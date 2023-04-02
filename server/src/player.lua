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

local _RSVD_NAME_eventTriggers = {}
function addTrigger(triggerType, callback)
    assertType(triggerType, 'string')
    assertType(callback, 'function')

    if not _RSVD_NAME_eventTriggers[triggerType] then
        _RSVD_NAME_eventTriggers[triggerType] = {}
    end

    table.insert(_RSVD_NAME_eventTriggers[triggerType], callback)
end

function _RSVD_NAME_runScriptEventTrigger(triggerType, ...)
end

for triggerType = SYS_ON_BEGIN, (SYS_ON_END - 1)
do
    addTrigger(triggerType, function(...)
        for _, questUID in ipairs(_RSVD_NAME_callFuncCoop('queryQuestTriggerList', triggerType)) do
            -- don't use uidExecute(questUID, ...) here
            -- player shall not assume the trigger callback function name in target quest

            -- post trigger parameter to quest
            -- won't wait for trigger done, don't assume target quest will reply
            runQuestTrigger(questUID, triggerType, ...)
        end
    end)
end

--
-- )###"
