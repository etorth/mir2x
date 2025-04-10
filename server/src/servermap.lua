--, u8R"###(
--

-- only map can use this
-- because servermap re-run the coroutine peroidically

function asyncWait(ms)
    local start = getTime()
    while getTime() < start + ms
    do
        coroutine.yield()
    end
end

function addGuard(guard, x, y, dir)
    assertType(guard, 'integer', 'string')
    assertType(    x, 'integer')
    assertType(    y, 'integer')
    assertType(  dir, 'integer')

    return _RSVD_NAME_callFuncCoop('addGuard', guard, x, y, dir)
end

--
-- )###"
