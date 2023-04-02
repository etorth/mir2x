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

local _RSVD_NAME_questTriggers = {}
function addQuestTrigger(triggerType, callback)
    assertType(triggerType, 'integer')
    assertType(callback, 'function')

    if triggerType < SYS_ON_BEGIN or event >= SYS_ON_END then
        fatalPrintf('Invalid trigger type: %d', triggerType)
    end

    if not _RSVD_NAME_questTriggers[triggerType] then
        _RSVD_NAME_questTriggers[triggerType] = {}
        _RSVD_NAME_callFuncCoop('modifyQuestTriggerType', triggerType, true)
    end

    table.insert(_RSVD_NAME_questTriggers[triggerType], callback)
end

function _RSVD_NAME_callTriggers(triggerType, uid, ...)
    assertType(triggerType, 'integer')
    assertType(uid, 'integer')

    if triggerType < SYS_ON_BEGIN or event >= SYS_ON_END then
        fatalPrintf('Invalid trigger type: %d', triggerType)
    end

    local args = {...}
    if triggerType == SYS_ON_LEVELUP then
        assertType(args[1], 'integer')
        assertType(args[2], 'integer')
        assertType(args[3], 'nil')

        assert(args[1] < args[2])
        if _RSVD_NAME_callTriggers[SYS_ON_LEVELUP] then
            local doneKeyList = {}
            for triggerKey, triggerFunc in pairs(_RSVD_NAME_callTriggers[SYS_ON_LEVELUP]) do
                local result = triggerFunc(uid, args[1], args[2])
                if type(result) == 'boolean' then
                    if result then
                        table.insert(doneKeyList, triggerKey)
                    end
                elseif type(result) ~= 'nil' then
                    table.insert(doneKeyList, triggerKey)
                    addLog(LOGTYPE_WARNING, 'Trigger %s returns invalid type %s, trigger removed.', tostring(triggerKey), type(result))
                end

                for _, key in ipairs(doneKeyList) do
                    _RSVD_NAME_callTriggers[SYS_ON_LEVELUP][key] = nil
                end

                if tableEmpty(_RSVD_NAME_callTriggers[SYS_ON_LEVELUP]) then
                    _RSVD_NAME_callTriggers[SYS_ON_LEVELUP] = nil
                    deleteQuestTriggerEvent(SYS_ON_LEVELUP)
                end
            end
        end
    elseif triggerType == SYS_ON_KILL then
    else
        fatalPrintf('Invalid trigger type: %d', triggerType)
    end
end

--
-- )###"
