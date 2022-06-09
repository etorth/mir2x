--, u8R"###(
--

function sendExecute(uid, code)
    sendCallStackExecute(getTLSTable().uid, uid, code)
end

-- send lua code to uid to execute
-- used to support complicated logic through actor message
function uidExecute(uid, code)
    assertType(code, 'string')
    sendExecute(uid, code)

    local resList = {waitEvent()}
    if resList[1] ~= uid then
        fatalPrintf('Send lua code to uid %s but get response from %d', uid, resList[1])
    end

    if resList[2] ~= SYS_EXECDONE then
        fatalPrintf('Wait event as SYS_EXECDONE but get %s', resList[2])
    end

    return table.unpack(resList, 3)
end

function sendQuery(uid, query)
    sendCallStackQuery(getTLSTable().uid, uid, query)
end

function waitEvent()
    while true do
        local resList = {pollCallStackEvent(getTLSTable().uid)}
        if next(resList) == nil then
            coroutine.yield()
        else
            local from  = resList[1]
            local event = resList[2]

            assertType(from, 'integer')
            assertType(event, 'string')

            return table.unpack(resList)
        end
    end
end

function uidQueryString(uid, query)
    sendQuery(uid, query)
    local from, event, value = waitEvent()

    if from ~= uid then
        fatalPrintf('Send query to uid %s but get response from %s', uid, from)
    end

    if event ~= SYS_NPCQUERY then
        fatalPrintf('Wait event as SYS_NPCQUERY but get %s', tostring(event))
    end
    return value
end

function uidQuery(uid, query, ...)
    return uidQueryString(uid, query:format(...))
end

function uidQueryName(uid)
    return uidExecute(uid, [[ return getName() ]])
end

function uidQueryRedName(uid)
    return false
end

function uidQueryLevel(uid)
    return tonumber(uidQuery(uid, 'LEVEL'))
end

function uidQueryGold(uid)
    return uidExecute(uid, [[ return getGold() ]])
end

function convItemSeqID(item)
    if math.type(item) == 'integer' then
        return item, 0
    elseif type(item) == 'string' then
        return getItemID(item), 0
    elseif type(item) == 'table' then
        if item.itemID ~= nil then
            local itemID = convItemSeqID(item.itemID)
            if item.seqID == nil then
                return itemID, 0
            elseif math.type(item.seqID) == 'integer' then
                return itemID, item.seqID
            elseif type(item.seqID) == 'string' then
                return itemID, tonumber(item.seqID)
            else
                fatalPrintf("invalid augument: unexpected item.seqID type: %s", type(item.seqID))
            end
        else
            fatalPrintf("invalid augument: table has no itemID")
        end
    else
        fatalPrintf('invalid argument: item = %s', tostring(item))
    end
end

function uidRemove(uid, item, count)
    local itemID, seqID = convItemSeqID(item)
    if itemID == 0 then
        fatalPrintf('invalid item: %s', tostring(item))
    end

    local value = uidQuery(uid, 'REMOVE %d %d %d', itemID, seqID, argDefault(count, 1))
    if value == '1' then
        return true
    elseif value == '0' then
        return false
    else
        fatalPrintf('invalid query result: %s', value)
    end
end

-- always use 金币（小）to represent the gold item
-- when convert to a SDItem the real 小中大 will get figured out by the count
function uidRemoveGold(uid, count)
    return uidRemove(uid, '金币（小）', count)
end

function uidSecureItem(uid, itemID, seqID)
    local result = uidQuery(uid, 'SECURE %d %d', itemID, seqID)
    if result == '1' then
        return true
    elseif result == '0' then
        return false
    else
        fatalPrintf('invalid query result: %s', result)
    end
end

function uidShowSecuredItemList(uid)
    local result = uidQuery(uid, 'SHOWSECURED')
    if result == '1' then
        return true
    elseif result == '0' then
        return false
    else
        fatalPrintf('invalid query result: %s', result)
    end
end

function uidGrant(uid, item, count)
    local itemID = convItemSeqID(item)
    if itemID == 0 then
        fatalPrintf('invalid item: %s', tostring(item))
    end

    local value = uidQuery(uid, 'GRANT %d %d', itemID, argDefault(count, 1))
    if value == '1' then
        return true
    elseif value == '0' then
        return false
    else
        fatalPrintf('invalid query result: %s', value)
    end
end

-- call stack get cleaned after one processNPCEvent call
-- if script needs to transfer information between call stacks, use this table, which uses uid as key
--
-- drawback: it's hard to clear the table
-- because players are not expected to inform NPCs that they are to be offline
-- uid outside of NPC's view can trigger to call clearGlobalTable(uid) but it's not a gentle way

-- use upvalue than global variable
-- otherwise when update this table the real update happens in TLS table

-- TODO
-- use uidRead('var_name'), uidWrite('var_name', var) to replace this
-- keep information in uid object, not in NPC table, then there is no life cycle issue
local g_uidGlobalTableList = {}
function getGlobalTable(uid)
    if uid ~= nil then
        assertType(uid, 'integer')
    else
        uid = getTLSTable().uid
    end

    if not g_uidGlobalTableList[uid] then
        g_uidGlobalTableList[uid] = {}
    end
    return g_uidGlobalTableList[uid]
end

-- clean a global table of a uid
-- NPC needs a better way to trigger to delete the uid global table
--
-- support clearGlobalTable()                   : uid = getTLSTable().uid, clearElemOnly = true
-- support clearGlobalTable(uid)                :                          clearElemOnly = true
-- support clearGlobalTable(clearElemOnly)      : uid = getTLSTable().uid
-- support clearGlobalTable(uid, clearElemOnly) :

function clearGlobalTable(arg1, arg2)
    local uid = nil
    local clearElemOnly = nil

    if arg1 == nil then
        uid = getTLSTable().uid
        clearElemOnly = true
    elseif arg2 == nil then
        if type(arg1) == 'number' and math.type(arg1) == 'integer' then
            uid = arg1
            clearElemOnly = true
        elseif type(arg1) == 'boolean' then
            uid = getTLSTable().uid
            clearElemOnly = arg1
        else
            fatalPrintf('invalid argument type: %s', type(arg1))
        end
    else
        assertType(arg1, 'integer')
        assertType(arg2, 'boolean')

        uid = arg1
        clearElemOnly = arg2
    end

    if g_uidGlobalTableList[uid] ~= nil then
        if clearElemOnly then
            for k, v in pairs(g_uidGlobalTableList[uid]) do
                g_uidGlobalTableList[uid][k] = nil
            end
        else
            g_uidGlobalTableList[uid] = nil
        end
    end
end

function uidGrantGold(uid, count)
    uidGrant(uid, '金币（小）', count)
end

function uidPostXML(uid, xmlFormat, ...)
    if type(uid) ~= 'number' or type(xmlFormat) ~= 'string' then
        fatalPrintf("invalid argument type: uid: %s, xmlFormat: %s", type(uid), type(xmlFormat))
    end
    uidPostXMLString(uid, xmlFormat:format(...))
end

function has_processNPCEvent(verbose, event)
    verbose = verbose or false
    if type(verbose) ~= 'boolean' then
        verbose = false
        addLog(LOGTYPE_WARNING, 'parmeter verbose is not boolean type, assumed false')
    end

    if type(event) ~= 'string' then
        event = nil
        addLog(LOGTYPE_WARNING, 'parmeter event is not string type, ignored')
    end

    if not processNPCEvent then
        if verbose then
            addLog(LOGTYPE_WARNING, "NPC %s: processNPCEvent is not defined", getNPCFullName())
        end
        return false
    elseif type(processNPCEvent) ~= 'table' then
        if verbose then
            addLog(LOGTYPE_WARNING, "NPC %s: processNPCEvent is not a function table", getNPCFullName())
        end
        return false
    else
        local count = 0
        for _ in pairs(processNPCEvent) do
            -- here for each entry we can check if the key is string and value is function type
            -- but can possibly be OK if the event is not triggered
            count = count + 1
        end

        if count == 0 then
            if verbose then
                addLog(LOGTYPE_WARNING, "NPC %s: processNPCEvent is empty", getNPCFullName())
            end
            return false
        end

        if not event then
            return true
        end

        if type(processNPCEvent[event]) ~= 'function' then
            if verbose then
                addLog(LOGTYPE_WARNING, "NPC %s: processNPCEvent[%s] is not a function", getNPCFullName(), event)
            end
            return false
        end
        return true
    end
end

-- entry coroutine for event handling
-- it's event driven, i.e. if the event sink has no event, this coroutine won't get scheduled

function coth_main(uid)
    -- setup current call stack uid
    -- all functions in current call stack can use this implicit argument as *this*
    getTLSTable().uid = uid
    getTLSTable().startTime = getNanoTstamp()

    -- poll the event sink
    -- current call stack only process 1 event and then clean itself
    local from, event, value = waitEvent()
    if event ~= SYS_NPCDONE then
        if has_processNPCEvent(false, event) then
            processNPCEvent[event](from, value)
        else
            -- don't exit this loop
            -- always consume the event no matter if the NPC can handle it
            uidPostXML(uid,
            [[
                <layout>
                    <par>我听不懂你在说什么。。。</par>
                    <par></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE)
        end
    end

    -- event process done
    -- clean the call stack itself, next event needs another call stack
    clearTLSTable()
end

--
-- )###"
