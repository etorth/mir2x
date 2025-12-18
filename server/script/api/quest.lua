local quest = {}

function quest.getName(questUID)
    assertType(questUID, "integer")
    assert(isQuest(questUID))
    return uidRemoteCall(questUID, [[ return getQuestName() ]])
end

function quest.getUID(questName)
    assertType(questName, "string")
    assert(#questName > 0)
    return _RSVD_NAME_callFuncCoop('queryQuestUID', questName)
end

function quest.getState(questUID, fargs)
    assertType(questUID, "integer")
    assert(isQuest(questUID))

    assertType(fargs, "table")

    assertType(fargs.uid, 'integer')
    assert(isPlayer(fargs.uid))

    assertType(fargs.fsm, 'string', 'nil')

    return uidRemoteCall(questUID, fargs,
    [[
        local fargs = ...
        return dbGetQuestState(fargs.uid, fargs.fsm)
    ]])
end

function quest.setState(questUID, fargs)
    assertType(questUID, "integer")
    assert(isQuest(questUID))

    assertType(fargs, 'table')
    return uidRemoteCall(questUID, fargs,
    [[
        local fargs = ...
        return setQuestState(fargs)
    ]])
end

function quest.setDesp(questUID, fargs)
    assertType(questUID, "integer")
    assert(isQuest(questUID))

    assertType(fargs, 'table')
    return uidRemoteCall(questUID, fargs,
    [[
        local fargs = ...
        return setQuestDesp(fargs)
    ]])
end

return quest
