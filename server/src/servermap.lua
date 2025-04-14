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

function getNPCharUID(npcName)
    assertType(npcName, 'string')
    return _RSVD_NAME_callFuncCoop('getNPCharUID', npcName)
end

--
-- )###"
