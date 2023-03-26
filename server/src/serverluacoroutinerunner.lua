--, u8R"###(
--

function _RSVD_NAME_luaCoroutineRunner_main(code)
    assertType(code, 'string')
    return (load(code))()
end

function _RSVD_NAME_callFuncCoop(funcName, ...)
    local done = nil
    local result = nil

    local function onOK(...)
        done = true
        result = {...}

        -- after this line
        -- C level will call resumeCORunner(threadKey) cooperatively
        -- this is black magic, we can not put resumeCORunner(threadKey) explicitly here in lua, see comments below:

        -- for callback function onOK and onError
        -- they should keep simple as above, only setup marks/flags

        -- don't call other complext functions which gives callback hell
        -- I design the player runner in sequential manner

        -- more importantly, don't do runner-resume in the onOK/onError callback
        -- which causes crash, because if we resume in onOK/onError, then when the callback gets triggeerred, stack is:
        --
        --   --C-->onOK/onError-->resumeCORunner(getTLSTable().threadKey)-->code in this runner after _RSVD_NAME_requestSpaceMove/coroutine.yield()
        --     ^       ^                ^                                   ^
        --     |       |                |                                   |
        --     |       |                |                                   +------ lua
        --     |       |                +------------------------------------------ C
        --     |       +----------------------------------------------------------- lua
        --     +------------------------------------------------------------------- C
        --
        -- see here C->lua->C->lua with yield/resume
        -- this crashes
    end

    local function onError()
        done = false
        -- transparent logic same as onOK
    end

    local args = {...}

    table.insert(args, onOK)
    table.insert(args, onError)
    table.insert(args, getTLSTable().threadKey)

    _G[string.format('_RSVD_NAME_%sCoop', funcName)](table.unpack(args))

    -- onOK/onError can get ran immedately in _RSVD_NAME_funcCoop
    -- in this situation we shall not yield

    if done == nil then
        coroutine.yield()
    end

    if result == nil then
        return nil
    else
        return table.unpack(result)
    end
end


local function _RSVD_NAME_waitRemoteCallResult()
    local seqID = getTLSTable().threadKey
    while true do
        local resList = {_RSVD_NAME_pollRemoteCallResult(seqID)}
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
local function _RSVD_NAME_uidExecute(uid, code)
    assertType(uid, 'integer')
    assertType(code, 'string')
    _RSVD_NAME_sendRemoteCall(getTLSTable().threadKey, uid, code)

    local resList = {_RSVD_NAME_waitRemoteCallResult()}
    if resList[1] ~= uid then
        fatalPrintf('Send lua code to uid %s but get response from %d', uid, resList[1])
    end

    if resList[2] ~= SYS_EXECDONE then
        fatalPrintf('Wait event as SYS_EXECDONE but get %s', resList[2])
    end

    return table.unpack(resList, 3)
end

function uidExecute(uid, code, ...)
    return _RSVD_NAME_uidExecute(uid, code:format(...))
end

--
-- )###"
