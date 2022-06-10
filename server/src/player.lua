--, u8R"###(
--

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    requestPause(ms, getTLSTable().runSeqID)
    coroutine.yield()
end

function spaceMove(mapID, x, y)
    local done = nil
    local function onOK()
        done = true
    end

    local function onError()
        done = false
    end

    requestSpaceMove(mapID, x, y, onOK, onError, getTLSTable().runSeqID)

    while done == nil do
        coroutine.yield()
    end
    return done
end

function coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

--
-- )###"
