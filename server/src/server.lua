--, u8R"###(
--

function addCWLog(logType, logPrompt, logString, ...)
    if type(logType) ~= 'number' or type(logPrompt) ~= 'string' or type(logString) ~= 'string' then
        fatalPrintf('invalid argument type: addCWLog(%s, %s, %s, ...)', type(logType), type(logPrompt), type(logString))
    end
    addCWLogString(logType, logPrompt, logString:format(...))
end

function listMap()
    local mapNameTable = {}
    for k, v in ipairs(getMapIDList()) do
        addCWLogString(0, ".", string.format('%s %s', tostring(v), getMapName(v)))
    end
end

--
-- )###"
