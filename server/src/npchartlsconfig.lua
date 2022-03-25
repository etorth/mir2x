--, u8R"###(
--

-- begin tls setup
-- from https://stackoverflow.com/a/24358483/1490269
-- setup all variable-access in thread as implicitly thread-local

local _G, coroutine = _G, coroutine
local ____g_mir2x_threadId, ____g_mir2x_inMainThread = coroutine.running()

if not ____g_mir2x_inMainThread then
    -- TODO error out
end

local ____g_mir2x_threadLocalTable = setmetatable({[____g_mir2x_threadId] = _G}, {__mode = "k"})
local ____g_mir2x_threadLocalMetaTable = {}

function ____g_mir2x_threadLocalMetaTable:__index(k)
    local currThreadId, currInMainThread = coroutine.running()
    if currInMainThread then
        -- TODO error out
    end

    local currThreadTable = ____g_mir2x_threadLocalTable[currThreadId]
    if currThreadTable then
        return currThreadTable[k]
    else
        return _G[k]
    end
end

function ____g_mir2x_threadLocalMetaTable:__newindex(k, v)
    local currThreadId, currInMainThread = coroutine.running()
    if currInMainThread then
        -- TODO error out
    end

    local currThreadTable = ____g_mir2x_threadLocalTable[currThreadId]
    if not currThreadTable then
        currThreadTable = setmetatable({ _G = _G}, {__index = _G})
        ____g_mir2x_threadLocalTable[currThreadId] = currThreadTable
    end
    currThreadTable[k] = v
end

-- convenient access to thread local variables via the `____g_mir2x_TLENV` table:
-- user can use ____g_mir2x_TLENV.var to explicitly declare a thread local varible and access it
____g_mir2x_TLENV = setmetatable({}, ____g_mir2x_threadLocalMetaTable)

-- change default lua env
-- makes all variables implicitly thread local
_ENV = ____g_mir2x_TLENV

-- end tls setup
-- following function variable access always goes into tls first

--
-- )###"
