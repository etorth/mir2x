--, u8R"###(
--

LOGTYPE_INFO    = 0
LOGTYPE_WARNING = 1
LOGTYPE_FATAL   = 2
LOGTYPE_DEBUG   = 3

-- example:
--     addLog(LOGTYPE_INFO, 'hello world')
--     addLog(LOGTYPE_INFO, 'hello world: %d', 12)
--
function addLog(logType, logString, ...)
    if type(logType) ~= 'number' or type(logString) ~= 'string' then
        error(string.format('invalid argument type: addLog(%s, %s, ...)', type(logType), type(logString)))
    end
    addLogString(logType, logString:format(...))
end

function errorPrintf(s, ...)
    if type(s) ~= 'string' then
        error(string.format('invalid argument type: errorPrintf(%s, ...)', type(s)))
    end
    error(s:format(...))
end

function getFileName()
    return debug.getinfo(2, 'S').short_src
end

function getBackTraceLine()
    local info = debug.getinfo(3, "Sl")

    -- check if the backtracing info valid
    -- if not valid we return a empty string to addLog()

    if not info then
        return ""
    end

    -- if it's invoked by a C function like:
    --     LuaModule["addLog"]("hello world")
    -- then return "C_FUNC"

    if info.what == "C" then
        return "C_FUNC"
    end

    -- invoked from a lua function
    -- return the invocation layer information

    return string.format("[%s]: %d", info.short_src, info.currentline)
end

function addExtLog(logType, logInfo)

    -- add type checking here
    -- need logType as int and logInfo as string

    if type(logType) == 'number' and type(logInfo) == 'string' then
        addLog(logType, getBackTraceLine() .. ': ' .. logInfo)
        return
    end

    -- else we need to give warning
    addLog(1, 'addExtLog(logType: int, logInfo: string)')
end

--
-- )###"
