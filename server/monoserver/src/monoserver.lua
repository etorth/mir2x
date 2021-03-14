--, u8R"###(
--
function listMap()
    local mapNameTable = {}
    for k, v in ipairs(getMapIDList()) do
        addCWLogString(0, ".", string.format('%s %s', tostring(v), mapID2Name(v)))
    end
end

g_helpTable = {}
g_helpTable["listMap"] = "print all map indices to current window"

function help(queryKey)
    if g_helpTable[queryKey] then
        addCWLogString(0, "> ", g_helpTable[queryKey])
    else
        addCWLogString(2, "> ", "No registered help information for input")
    end
end
--
-- )###"
