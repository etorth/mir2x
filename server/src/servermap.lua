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

--
-- )###"
