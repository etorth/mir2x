--, u8R"###(
--

function dbExec(cmd, ...)
    dbExecString(cmd:format(...))
end

function dbQuery(query, ...)
    return dbQueryString(query:format(...))
end

--
-- )###"
