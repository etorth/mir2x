--, u8R"###(
--

-- example:
--     addLog(LOGTYPE_INFO, 'hello world')
--     addLog(LOGTYPE_INFO, 'hello world: %d', 12)
--
function addLog(logType, logString, ...)
    if type(logType) ~= 'number' or type(logString) ~= 'string' then
        error(string.format('Invalid argument type: addLog(%s, %s, ...)', type(logType), type(logString)))
    end
    addLogString(logType, logString:format(...))
end

function fatalPrintf(s, ...)
    if type(s) ~= 'string' then
        error(string.format('Invalid argument type: fatalPrintf(%s, ...)', type(s)))
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
    local typestrs = table.pack(...)
    for i = 1, typestrs.n do
        local typestr = typestrs[i]
        if type(typestr) ~= 'string' then
            fatalPrintf('Invalid type string, expect string, get %s', type(typestr))
        end

        if type(var) == 'number' then
            if math.type(var) == typestr then
                return var
            end

        elseif typestr == 'array' then
            if isArray(var) then
                return var
            end

        else
            if type(var) == typestr then
                return var
            end
        end
    end

    if typestrs.n == 0 then
        fatalPrintf('Invalid argument: no type string provided')

    elseif typestrs.n == 1 then
        fatalPrintf('Assertion failed: expect %s, get %s', typestrs[1], type(var))

    else
        local i = 1
        local errStrs = {}

        table.insert(errStrs, 'Assertion failed: expect')
        while i <= typestrs.n - 2 do
            table.insert(errStrs, string.format(' %s,', typestrs[i]))
            i = i + 1
        end

        table.insert(errStrs, string.format(' %s or %s, get %s', typestrs[typestrs.n - 1], typestrs[typestrs.n], type(var)))
        fatalPrintf(table.concat(errStrs))
    end
end

function assertValue(var, ...)
    local args = table.pack(...)
    if args.n == 0 then
        fatalPrintf('Invalid argument: no value provided')
    end

    for i = 1, args.n do
        if var == args[i] then
            return var
        end
    end
    fatalPrintf('Assertion failed: expect %s, get %s', table.concat(args, ', ', 1, args.n), tostring(var))
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

local _RSVD_NAME_canSerializeSimpleTypes = {
    ['nil'    ] = true,
    ['number' ] = true,
    ['string' ] = true,
    ['boolean'] = true,
}
function asInitString(var)
    -- for following code:
    --
    --    u = any_func()
    --    load(string.format([[v = %s]], asInitString(u)))()
    --
    -- then u and v should be conceptually identical

    -- can not support functions
    -- can not support metatables
    -- can not support cyclic reference

    if _RSVD_NAME_canSerializeSimpleTypes[type(var)] then
        return string.format('%q', var)

    elseif isArray(var) then
        local strs = {}
        for _, v in ipairs(var) do
            table.insert(strs, asInitString(v))
        end
        return '{' .. table.concat(strs, ',') .. '}'

    elseif type(var) == 'table' then
        local strs = {}
        for k, v in pairs(var) do
            table.insert(strs, string.format('[%s]=%s', asInitString(k), asInitString(v)))
        end
        return '{' .. table.concat(strs, ',') .. '}'

    else
        fatalPrintf('Invalid type: %s', type(var))
    end
end

function canSerialize(var)
    if _RSVD_NAME_canSerializeSimpleTypes[type(var)] then
        return true

    elseif isArray(var) then
        if getmetatable(var) then
            return false
        end

        for _, v in ipairs(var) do
            if not canSerialize(v) then
                return false
            end
        end
        return true

    elseif type(var) == 'table' then
        if getmetatable(var) then
            return false
        end

        for k, v in pairs(var) do
            if not canSerialize(k) or not canSerialize(v) then
                return false
            end
        end
        return true

    else
        return false
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

local _RSVD_NAME_cachedROTable = setmetatable({}, { __mode = "k" })
function rotable(arg)
    if type(arg) ~= "table" then
        return arg
    end

    local cached = _RSVD_NAME_cachedROTable[arg]
    if not cached then
        cached = setmetatable({},
        {
            __index = function(_, k)
                return rotable(arg[k])
            end,

            __newindex = function()
                error("Attempt to modify a read-only table", 2)
            end,

            __pairs = function()
                return function(_, i)
                    local k, v = next(arg, i)
                    return k, rotable(v)
                end, nil, nil
            end,

            __ipairs = function()
                return function(_, i)
                    return i + 1, rotable(arg[i + 1])
                end, nil, 0
            end,

            __len = function()
                return #arg
            end,
        })
        _RSVD_NAME_cachedROTable[arg] = cached
    end
    return cached
end

function tableEmpty(t, allowNil)
    if t == nil and argDefault(allowNil, true) then
        return true
    elseif type(t) == 'table' then
        return next(t) == nil
    else
        fatalPrintf('Invalid argument: tableEmpty(%s)', type(t))
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

    fatalPrintf('Invalid argument type: %s', type(t))
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
