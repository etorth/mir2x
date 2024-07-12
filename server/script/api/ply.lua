local ply = {}

function ply.getLevel(uid) return uidRemoteCall(uid, [[ return getLevel() ]]) end
function ply.getGold (uid) return uidRemoteCall(uid, [[ return getGold () ]]) end
function ply.getGener(uid) return uidRemoteCall(uid, [[ return getGener() ]]) end
function ply.getName (uid) return uidRemoteCall(uid, [[ return getName () ]]) end

function ply.getTeamLeader    (uid) return uidRemoteCall(uid, [[ return getTeamLeader    () ]]) end
function ply.getTeamMemberList(uid) return uidRemoteCall(uid, [[ return getTeamMemberList() ]]) end

function ply.dbHasFlag   (uid, flag) return uidRemoteCall(uid, flag, [[ return dbHasFlag   (...) ]]) end
function ply.dbAddFlag   (uid, flag) return uidRemoteCall(uid, flag, [[ return dbAddFlag   (...) ]]) end
function ply.dbRemoveFlag(uid, flag) return uidRemoteCall(uid, flag, [[ return dbRemoveFlag(...) ]]) end

function ply.getWLItem(uid, wlType) return uidRemoteCall(uid, wlType, [[ return getWLItem(...)]]) end

function ply.postString(uid, msg, ...)
    local args = table.pack(...)
    uidRemoteCall(uid, msg, args,
    [[
        local msg, args = ...
        return postString(msg, table.unpack(args, 1, args.n))
    ]])
end

function ply.addItem(uid, item, count)
    assertType(uid, 'integer')
    assert(isPlayer(uid))

    assertType(item, 'string', 'integer')
    if type(item) == 'string' then
        item = getItemID(item)
    end
    assert(item > 0)

    assertType(count, 'integer', 'nil')
    if count == nil then
        count = 1
    end
    assert(count > 0)

    return uidRemoteCall(uid, item, count,
    [[
        local item, count = ...
        return addItem(item, count)
    ]])
end

function ply.removeItem(uid, item, arg1, arg2)
    assertType(uid, 'integer')
    assert(isPlayer(uid))

    assertType(item, 'string', 'integer')

    if type(item) == 'string' then
        item = getItemID(item)
    end

    assert(item > 0)

    local seq
    local count

    if math.type(arg1) == 'integer' and math.type(arg2) == 'integer' then
        seq = arg1
        count = arg2
    elseif math.type(arg1) == 'integer' and arg2 == nil then
        seq = 0
        count = arg1
    end

    assert(seq >= 0)
    assert(count > 0)

    return uidRemoteCall(uid, item, seq, count,
    [[
        local item, seq, count = ...
        return removeItem(item, seq, count)
    ]])
end

function ply.hasItem(uid, item, arg1, arg2)
    assertType(uid, 'integer')
    assert(isPlayer(uid))

    assertType(item, 'string', 'integer')

    if type(item) == 'string' then
        item = getItemID(item)
    end

    assert(item > 0)

    local seq
    local count

    if math.type(arg1) == 'integer' and math.type(arg2) == 'integer' then
        seq = arg1
        count = arg2
    elseif math.type(arg1) == 'integer' and arg2 == nil then
        seq = 0
        count = arg1
    elseif arg1 == nil and arg2 == nil then
        seq = 0
        count = 1
    else
        fatalPrintf('Invalid arguments: %s, %s', type(arg1), type(arg2))
    end

    assert(seq >= 0)
    assert(count > 0)

    return uidRemoteCall(uid, item, seq, count,
    [[
        local item, seq, count = ...
        return hasItem(item, seq, count)
    ]])
end

function ply.spaceMove(playerUID, ...)
    assert(isPlayer(playerUID))
    local args = table.pack(...)

    uidRemoteCall(playerUID, args,
    [[
        local args = ...
        return spaceMove(table.unpack(args, 1, args.n))
    ]])
end

function ply.getQuestState(uid, ...)
    return uidRemoteCall(uid, table.pack(...),
    [[
        local args = ...
        return getQuestState(table.unpack(args, 1, args.n))
    ]])
end

function ply.addTrigger(playerUID, triggerType, funcStr)
    assertType(playerUID, 'integer')
    assert(isPlayer(playerUID))

    assertType(triggerType, 'integer')
    assertType(funcStr, 'string')

    return uidRemoteCall(playerUID, triggerType, funcStr,
    [[
        local triggerType, funcStr = ...
        return addTrigger(triggerType, load(funcStr))
    ]])
end

return ply
