--, u8R"###(
--

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    RSVD_NAME_requestPause(ms, getTLSTable().runSeqID)
    coroutine.yield()
end

function spaceMove(mapID, x, y)
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
        --   --C-->onOK/onError-->resumeCORunner(getTLSTable().runSeqID)-->code in this runner after RSVD_NAME_requestSpaceMove/coroutine.yield()
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

    RSVD_NAME_requestSpaceMove(mapID, x, y, onOK, onError, getTLSTable().runSeqID)

    coroutine.yield()
    return done
end

function coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

--
-- )###"
