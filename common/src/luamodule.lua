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

function fatalPrintf(s, ...)
    if type(s) ~= 'string' then
        error(string.format('invalid argument type: fatalPrintf(%s, ...)', type(s)))
    end
    error(s:format(...))
end

function argDef(arg, def)
    if arg == nil then
        assert(def ~= nil)
        return def
    else
        return arg
    end
end

function asString(arg)
    typeStr = type(arg)
    if typeStr == 'nil' or typeStr == 'number' or typeStr == 'boolean' or typeStr == 'string' then
        return scalarAsString(arg)
    elseif typeStr == 'table' then
        local convTable = {}
        for k, v in pairs(arg) do
            convTable[asString(k)] = asString(v)
        end
        return convTableAsString(convTable)
    else
        fatalPrintf('asString(%s) type not supported: %s', tostring(arg), type(arg))
    end
end

function fromString(s)
    if type(s) ~= 'string' then
        fatalPrintf('fromString(%s) expecting string, provided %s', tostring(s), type(s))
    end

    if string.sub(s, -1) == 't' then -- conv_table
        local convTable = convTableFromString(s)
        local realTable = {}
        for k, v in pairs(convTable) do
            realTable[fromString(asBinaryString(k))] = fromString(v)
            -- realTable[fromString(k)] = fromString(v)
        end
        return realTable
    else
        return scalarFromString(s)
    end
end

function asyncWait(ms)
    local start = getTime()
    while getTime() < start + ms
    do
        coroutine.yield()
    end
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
