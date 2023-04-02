--, u8R"###(
--

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
    assertType(uid, 'integer')

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
            _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', SYS_ON_LEVELUP, false)
        end
    elseif triggerType == SYS_ON_KILL then
        assertType(args[1], 'integer')
        assertType(args[2], 'nil')

        if _RSVD_NAME_triggers[SYS_ON_KILL] then
            local doneKeyList = {}
            for triggerKey, triggerFunc in pairs(_RSVD_NAME_triggers[SYS_ON_KILL]) do
                local result = triggerFunc(uid, args[1])
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
                _RSVD_NAME_triggers[SYS_ON_KILL][key] = nil
            end
        end

        if tableEmpty(_RSVD_NAME_triggers[SYS_ON_KILL]) then
            _RSVD_NAME_triggers[SYS_ON_KILL] = nil
            _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', SYS_ON_KILL, false)
        end
    else
        fatalPrintf('Invalid trigger type: %d', triggerType)
    end
end

--
-- )###"
