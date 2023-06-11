--, u8R"###(
--

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

function argDefault(arg, def)
    if arg == nil then
        assert(def ~= nil)
        return def
    else
        return arg
    end
end

function assertType(var, ...)
    local typestrs = {...}
    for _, typestr in ipairs(typestrs) do
        if type(typestr) ~= 'string' then
            fatalPrintf('invalid type string, expect string, get %s', type(typestr))
        end

        if type(var) == 'number' then
            if math.type(var) == typestr then
                return var
            end
        else
            if type(var) == typestr then
                return var
            end
        end
    end

    if #typestrs == 0 then
        fatalPrintf('invalid argument: no type string provided')

    elseif #typestrs == 1 then
        fatalPrintf('assertion failed: expect %s, get %s', typestrs[1], type(var))

    else
        local i = 1
        local errStrs = {}

        table.insert(errStrs, 'assertion failed: expect')
        while i <= #typestrs - 2 do
            table.insert(errStrs, string.format(' %s,', typestrs[i]))
            i = i + 1
        end

        table.insert(errStrs, ' %s or %s, get %s', typestrs[#typestrs - 1], typestrs[#typestrs], type(var))
        fatalPrintf(table.concat(errStrs))
    end
end

function assertValue(var, value)
    if type(value) == 'table' then
        for _, v in ipairs(value) do
            if var == v then
                return var
            end
        end
        fatalPrintf('assertion failed: expect %s, get %s', table.concat(value, ', '), tostring(var))
    else
        if var == value then
            return var
        end
        fatalPrintf('assertion failed: expect %s, get %s', tostring(value), tostring(var))
    end
end

function shuffleArray(arr)
    assert(isArray(arr))
    local shuffled = {}
    for _, v in ipairs(arr) do
        table.insert(shuffled, math.random(1, #shuffled + 1), v)
    end
    return shuffled
end

function isArray(tbl)
    if type(tbl) ~= 'table' then
        return false
    end

    local i = 0
    for _ in pairs(tbl) do
        i = i + 1
        if tbl[i] == nil then
            return false
        end
    end
    return true
end

function asInitString(var)
    if type(var) == 'boolean' then
        return tostring(var)

    elseif type(var) == 'string' then
        return [[']] .. var .. [[']]

    elseif math.type(var) == 'integer' then
        return tostring(var)

    elseif isArray(var) then
        local strs = {}
        for _, v in ipairs(var) do
            table.insert(strs, asInitString(v))
        end
        return '{' .. table.concat(strs, ',') .. '}'

    else
        fatalPrintf('Invalid type: %s', type(var))
    end
end

function hasChar(s)
    if s == nil then
        return false
    end

    assertType(s, 'string')
    return string.len(s) > 0
end

function splitString(str, sep)
    assertType(str, 'string')
    assertType(sep, 'string', 'nil')

    if sep == nil then
        sep = "%s"
    end

    local result = {}
    for s in string.gmatch(str, '([^' .. sep .. ']+)') do
        table.insert(result, s)
    end
    return result
end

function convItemSeqID(item)
    if math.type(item) == 'integer' then
        if item >= 0 then
            return item, 0
        else
            fatalPrintf("Invalid argument: item = %s", tostring(item))
        end

    elseif type(item) == 'string' then
        return getItemID(item), 0

    elseif isArray(item) then
        local itemID = convItemSeqID(item[1])
        if item[2] == nil then
            return itemID, 0

        elseif math.type(item[2]) == 'integer' then
            if item[2] >= 0 then
                return itemID, item[2]
            else
                fatalPrintf('Invalid argument: {%d, %d}', itemID, item[2])
            end

        else
            fatalPrintf('Invalid argument: {%d, %s}', itemID, tostring(item[2]));
        end

    elseif type(item) == 'table' then
        return convItemSeqID(item.itemID, item.seqID)

    else
        fatalPrintf('Invalid argument: item = %s', tostring(item))
    end
end

function getFileName()
    return debug.getinfo(2, 'S').source
end

function rotable(tbl, recursive)
    assertType(tbl, 'table')
    if recursive ~= nil then
        assertType(recursive, 'boolean')
    end

    local function plain_rotable(tb)
        return setmetatable({}, {
            __index = tb,
            __newindex = function()
                error("attempt to update a read-only table")
            end,

            __pairs = function()
                return next, tb, nil
            end,

            __ipairs = function()
                local function iter(t, i)
                    local j = i + 1
                    local v = t[j]
                    if v ~= nil then
                        return j, v
                    end
                end
                return iter, tbl, 0
            end,

            __len = function()
                return #tb
            end
        })
    end

    local function dfs_rotable(tb)
        for k, v in pairs(tb) do
            if type(v) == 'table' then
                tb[k] = plain_rotable(dfs_rotable(v))
            end
        end
        return plain_rotable(tb)
    end

    if (recursive == nil or recursive) then
        return dfs_rotable(tbl)
    else
        return plain_rotable(tbl)
    end
end

function tableEmpty(t, allowNil)
    if t == nil and argDefault(allowNil, true) then
        return true
    elseif type(t) == 'table' then
        return next(t) == nil
    else
        fatalPrintf('invalid argument: tableEmpty(%s)', type(t))
    end
end

function tableSize(t, allowNil)
    if type(t) == 'table' then
        local size = 0
        for k, v in pairs(t) do
            size = size + 1
        end
        return size
    end

    if type(t) == 'nil' and argDefault(allowNil, true) then
        return 0
    end

    fatalPrintf('invalid argument type: %s', type(t))
end

tableLength = tableSize

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
