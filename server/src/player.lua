--, u8R"###(
--

function postString(msg, ...)
    postRawString(msg:format(...))
end

function call_RSVD_NAME_funcCoop(funcName, ...)
    local done = nil
    local function onOK()
        done = true
        -- after this line
        -- C level will call resumeCORunner(runSeqID) cooperatively
        -- this is black magic, we can not put resumeCORunner(runSeqID) explicitly here in lua, see comments below:

        -- for callback function onOK and onError
        -- they should keep simple as above, only setup marks/flags

        -- don't call other complext functions which gives callback hell
        -- I design the player runner in sequential manner

        -- more importantly, don't do runner-resume in the onOK/onError callback
        -- which causes crash, because if we resume in onOK/onError, then when the callback gets triggeerred, stack is:
        --
        --   --C-->onOK/onError-->resumeCORunner(getTLSTable().runSeqID)-->code in this runner after __RSVD_NAME_requestSpaceMove/coroutine.yield()
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
    table.insert(args, getTLSTable().runSeqID)

    _G[string.format('__RSVD_NAME_%sCoop', funcName)](table.unpack(args))

    -- onOK/onError can get ran immedately in __RSVD_NAME_funcCoop
    -- in this situation we shall not yield

    if done == nil then
        coroutine.yield()
    end

    return done
end

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    local oldTime = getTime()
    __RSVD_NAME_pauseYielding(ms, getTLSTable().runSeqID)
    return getTime() - oldTime
end

function randomMove()
    return call_RSVD_NAME_funcCoop('randomMove')
end

function spaceMove(mapID, x, y)
    return call_RSVD_NAME_funcCoop('spaceMove', mapID, x, y)
end

function __RSVD_NAME_coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

--
-- )###"
