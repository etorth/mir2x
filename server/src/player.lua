function dbHasFlag(flag)
    assertType(flag, 'string')
    local found, value = dbHasVar(flag)

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
    local fullMsg = msg:format(...)
    local fullPar = '<par>' .. fullMsg .. '</par>'

    if validPar(fullPar) then
        postParString(fullPar)
    else
        postRawString(fullMsg)
    end
end

function randomMove()
    return _RSVD_NAME_callFuncCoop('randomMove')
end

-- move the player, by map name, map id, or map uid
--
-- a name or an id only ever reaches the base copy of that map. pass the uid that
-- loadInstanceMap handed back to get into an instance copy — the C layer tells the two kinds
-- of integer apart by the type field, see uidf::isMap
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

-- run func on its own lua thread in this player, returns the key to cancel it by
--
-- same shape as runQuestThread: pause() inside the thread to delay the rest of it, and
-- closeThread(key) cancels the pause and the thread with it
--
--     local timer = runPlayerThread(function()
--         pause(3 * 60 * 1000)
--         postString('时间到了！')
--     end)
function runPlayerThread(func)
    assertType(func, 'function')

    local key = rollKey()
    runThread(key, func)
    return key
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

-- logs a lua error of one quest, the caller goes on with the other quests
local function logQuestError(msg, err)
    addLog(LOGTYPE_WARNING, '%s', msg)
    for line in tostring(err):gmatch('[^\n]+') do
        addLog(LOGTYPE_WARNING, '%s', line)
    end
end

function _RSVD_NAME_setupQuests()
    local questDespList = {}
    for _, questUID in ipairs(_RSVD_NAME_callFuncCoop('queryQuestUIDList') or {})
    do
        -- a quest failing its restore, i.e. a saved NPC behavior that doesn't load anymore, doesn't stop the other quests
        local restored, restoreErr = pcall(uidRemoteCall, questUID, getUID(),
        [[
            local playerUID = ...
            _RSVD_NAME_loadQuestContext(playerUID)
            _RSVD_NAME_restoreQuestStates(playerUID)
        ]])

        if not restored then
            logQuestError(string.format('Quest %s failed to restore player %s', getUIDString(questUID), getUIDString(getUID())), restoreErr)
        end

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
            -- a quest failing its trigger doesn't stop the other quests, i.e. their cleanup on SYS_ON_OFFLINE
            local ok, err = pcall(uidRemoteCall, questUID, triggerType, getUID(), table.pack(...),
            [[
                -- run quest triggers
                -- quest trigger and player triggers are designed to have identical parameters

                local triggerType, playerUID, packedArgs = ...
                _RSVD_NAME_trigger(triggerType, playerUID, table.unpack(packedArgs, 1, packedArgs.n))
            ]])

            if not ok then
                logQuestError(string.format('Quest %s failed to run trigger %d for player %s', getUIDString(questUID), triggerType, getUIDString(getUID())), err)
            end
        end
    end)
end
