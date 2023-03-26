--, u8R"###(
--

function postString(msg, ...)
    postRawString(msg:format(...))
end

function pause(ms)
    assertType(ms, 'integer')
    assert(ms >= 0)

    local oldTime = getTime()
    _RSVD_NAME_pauseYielding(ms, getTLSTable().threadKey)
    return getTime() - oldTime
end

function randomMove()
    return _RSVD_NAME_callFuncCoop('randomMove')
end

function spaceMove(mapID, x, y)
    return _RSVD_NAME_callFuncCoop('spaceMove', mapID, x, y)
end

function _RSVD_NAME_coth_runner(code)
    assertType(code, 'string')
    return (load(code))()
end

--
-- )###"
