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

function addMonster(monster, x, y, strictLoc)
    assertType(  monster, 'integer', 'string')
    assertType(        x, 'integer')
    assertType(        y, 'integer')
    assertType(strictLoc, 'boolean')
    return _RSVD_NAME_callFuncCoop('addMonster', monster, x, y, strictLoc)
end

--
-- )###"
