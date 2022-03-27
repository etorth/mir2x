--, u8R"###(
--

function sendQuery(uid, query)
    sendCallStackQuery(getCallStackTable().uid, uid, query)
end

function waitEvent()
    while true do
        local uid, event, value = pollCallStackEvent(getCallStackTable().uid)
        if uid then
            return uid, event, value
        end
        coroutine.yield()
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
    return uidQuery(uid, 'NAME')
end

function uidQueryRedName(uid)
    return false
end

function uidQueryLevel(uid)
    return tonumber(uidQuery(uid, 'LEVEL'))
end

function uidQueryGold(uid)
    return tonumber(uidQuery(uid, 'GOLD'))
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

    local value = uidQuery(uid, 'REMOVE %d %d %d', itemID, seqID, argDef(count, 1))
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

    local value = uidQuery(uid, 'GRANT %d %d', itemID, argDef(count, 1))
    if value == '1' then
        return true
    elseif value == '0' then
        return false
    else
        fatalPrintf('invalid query result: %s', value)
    end
end

-- setup call stack table for thread-based parameters
-- we spawn call stack by sol::thread which still access global table
-- so we can't have tls per call stack, have to save call stack related globals into this table
--
-- npchartlsconfig.lua provides a method for tls support
-- but it brings too much uncertainty
g_callStackTableList = {}

function getCallStackTable()
    local threadId, inMainThread = coroutine.running()
    if inMainThread then
        fatalPrintf('calling getCallStackTable() in main thead')
    end

    if not g_callStackTableList[threadId] then
        g_callStackTableList[threadId] = {}
    end
    return g_callStackTableList[threadId]
end

function clearCallStackTable()
    local threadId, inMainThread = coroutine.running()
    if inMainThread then
        fatalPrintf('calling clearCallStackTable() in main thead')
    end
    g_callStackTableList[threadId] = nil
end

-- call stack get cleaned after one processNPCEvent call
-- if script needs to transfer information between call stack, use this table, which uses uid as key
--
-- issue is it maybe hard to clear the table
-- because we are not expected to inform NPCs that an uid has offline
g_globalTableList = {}

function getGlobalTable()
    if not g_globalTableList[getCallStackTable().uid] then
        g_globalTableList[getCallStackTable().uid] = {}
    end
    return g_globalTableList[getCallStackTable().uid]
end

function clearGlobalTable()
    g_globalTableList[getCallStackTable().uid] = nil
end

function uidGrantGold(uid, count)
    uidGrant(uid, '金币（小）', count)
end

function uidPostXML(uid, xmlFormat, ...)
    if type(uid) ~= 'string' or type(xmlFormat) ~= 'string' then
        fatalPrintf("invalid argument type: uid: %s, xmlFormat: %s", type(uid), type(xmlFormat))
    end

    if not isUID(uid) then
        fatalPrintf("not a valid uid string: %s", uid)
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

function main(uid)
    -- setup current call stack uid
    -- all functions in current call stack can use this implicit argument as *this*
    getCallStackTable().uid = uid
    getCallStackTable().startTime = getNanoTstamp()

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
    clearCallStackTable()
end

--
-- )###"
