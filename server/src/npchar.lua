--, u8R"###(
--

function waitEvent()
    while true do
        local resList = {RSVD_NAME_pollCallStackEvent(getTLSTable().uid)}
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

-- send lua code to uid to execute
-- used to support complicated logic through actor message
function uidExecuteString(uid, code)
    assertType(uid, 'integer')
    assertType(code, 'string')
    RSVD_NAME_sendCallStackRemoteCall(getTLSTable().uid, uid, code, false)

    local resList = {waitEvent()}
    if resList[1] ~= uid then
        fatalPrintf('Send lua code to uid %s but get response from %d', uid, resList[1])
    end

    if resList[2] ~= SYS_EXECDONE then
        fatalPrintf('Wait event as SYS_EXECDONE but get %s', resList[2])
    end

    return table.unpack(resList, 3)
end

function uidExecute(uid, code, ...)
    return uidExecuteString(uid, code:format(...))
end

function uidSpaceMove(uid, map, x, y)
    local mapID = nil
    if type(map) == 'string' then
        mapID = getMapID(map)
    elseif math.type(map) == 'integer' and map >= 0 then
        mapID = map
    else
        fatalPrintf("Invalid argument: map = %s, x = %s, y = %s", map, x, y)
    end

    assertType(x, 'integer')
    assertType(y, 'integer')
    return uidExecute(uid, [[ return spaceMove(%d, %d, %d) ]], mapID, x, y)
end

function uidQueryName(uid)
    return uidExecute(uid, [[ return getName() ]])
end

function uidQueryRedName(uid)
    return false
end

function uidQueryLevel(uid)
    return uidExecute(uid, [[ return getLevel() ]])
end

function uidQueryGold(uid)
    return uidExecute(uid, [[ return getGold() ]])
end

function uidRemove(uid, item, count)
    local itemID, seqID = convItemSeqID(item)
    if itemID == 0 then
        fatalPrintf('invalid item: %s', tostring(item))
    end
    return uidExecute(uid, [[ return removeItem(%d, %d, %d) ]], itemID, seqID, argDefault(count, 1))
end

-- always use 金币（小）to represent the gold item
-- when convert to a SDItem the real 小中大 will get figured out by the count
function uidRemoveGold(uid, count)
    return uidRemove(uid, '金币（小）', count)
end

function uidSecureItem(uid, itemID, seqID)
    uidExecute(uid, [[ secureItem(%d, %d) ]], itemID, seqID)
end

function uidShowSecuredItemList(uid)
    uidExecute(uid, [[ reportSecuredItemList() ]])
end

function uidGrant(uid, item, count)
    local itemID = convItemSeqID(item)
    if itemID == 0 then
        fatalPrintf('invalid item: %s', tostring(item))
    end
    uidExecute(uid, [[ addItem(%d, %d) ]], itemID, argDefault(count, 1))
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

local RSVD_NAME_npcEventHandler = nil
function setEventHandler(eventHandler)
    if RSVD_NAME_npcEventHandler ~= nil then
        fatalPrintf('Call setEventHandler() twice')
    end

    assertType(eventHandler, 'table')
    if eventHandler[SYS_NPCINIT] == nil then
        fatalPrintf('Event handler does not support SYS_NPCINIT')
    end

    if type(eventHandler[SYS_NPCINIT]) ~= 'function' then
        fatalPrintf('Event handler for SYS_NPCINIT is not callable')
    end

    RSVD_NAME_npcEventHandler = eventHandler
end

function hasEventHandler(event)
    assert(hasChar(event))
    if RSVD_NAME_npcEventHandler == nil then
        return false
    end

    assertType(RSVD_NAME_npcEventHandler, 'table')
    if RSVD_NAME_npcEventHandler[event] == nil then
        return false
    end

    assertType(RSVD_NAME_npcEventHandler[event], 'function')
    return true
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

    assertType(from, 'integer')
    assertType(event, 'string')

    if event ~= SYS_NPCDONE then
        if hasEventHandler(event) then
            RSVD_NAME_npcEventHandler[event](from, value)
        else
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
