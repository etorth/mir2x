--, u8R"###(
--

function dbHasFlag(flag)
    assertType(flag, 'string')
    local found, value = dbGetVar(flag)

    if found then
        if value == SYS_FLAGVAL then
            return true
        else
            fatalPrintf('Not a flag name: %s', flag)
        end
    else
        assertType(value, 'nil')
        return false
    end
end

function dbAddFlag(flag)
    assertType(flag, 'string')
    return dbSetVar(flag, SYS_FLAGVAL)
end

function dbRemoveFlag(flag)
    assertType(flag, 'string')
    return dbRemoveVar(flag)
end

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

function getQuestState(questName, fsmName)
    assertType(questName, 'string')
    assertType(fsmName, 'string', 'nil')

    local questUID = _RSVD_NAME_callFuncCoop('queryQuestUID', questName)

    assertType(questUID, 'integer', 'nil')
    if questUID then
        assert(isQuest(questUID))
        return uidRemoteCall(questUID, getUID(), fsmName or SYS_QSTFSM,
        [[
            local playerUID, fsmName = ...
            return dbGetQuestState(playerUID, fsmName)
        ]])
    end
end

function _RSVD_NAME_setupQuests()
    local questDespList = {}
    for _, questUID in ipairs(_RSVD_NAME_callFuncCoop('queryQuestUIDList') or {})
    do
        uidRemoteCall(questUID, getUID(),
        [[
            local playerUID = ...

            local npcBehaviors = dbGetQuestField(playerUID, 'fld_npcbehaviors')
            assertType(npcBehaviors, 'table', 'nil')

            if npcBehaviors then
                for _, v in pairs(npcBehaviors) do
                    setupNPCQuestBehavior(v[1], v[2], playerUID, v[4], v[3])
                end
            end

            local states = _RSVD_NAME_dbGetQuestStateList(playerUID)
            assertType(states, 'table', 'nil')

            if states then
                assertType(states[SYS_QSTFSM], 'array')
                assertType(states[SYS_QSTFSM][1], 'string')

                if states[SYS_QSTFSM][1] ~= SYS_DONE then
                    for k, v in pairs(states) do
                        if v[1] ~= SYS_DONE then
                            runQuestThread(function()
                                _RSVD_NAME_enterQuestState(playerUID, k, v[1], v[2])
                            end)
                        end
                    end
                end
            end
        ]])

        local questName, questState, questDesp = uidRemoteCall(questUID, getUID(),
        [[
            local playerUID = ...
            return getQuestName(), dbGetQuestState(playerUID, SYS_QSTFSM), dbGetQuestDesp(playerUID)
        ]])

        assertType(questName,  'string')
        assertType(questState, 'string', 'nil')
        assertType(questDesp,  'table' , 'nil')

        if questState == SYS_DONE then
            questDespList[questName] = {[SYS_QSTFSM] = '任务已完成'}

        elseif questState then
            questDespList[questName] = questDesp or {}
        end
    end

    _RSVD_NAME_reportQuestDespList(questDespList)
end

function _RSVD_NAME_coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
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
