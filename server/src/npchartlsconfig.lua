--, u8R"###(
--

-- begin tls setup
-- from https://stackoverflow.com/a/24358483/1490269
-- setup all variable-access in thread as implicitly thread-local
--
-- this chunk is required to get sourced in main thread and before any call to main(uid)
-- ____g_mir2x_mainThreadId is always the main thread id

local _G, coroutine = _G, coroutine
local ____g_mir2x_mainThreadId, ____g_mir2x_inMainThread = coroutine.running()

-- put functions used in __index and __newindex into upvalue
-- otherwise lua search _ENV for them

local rawset = rawset
local setmetatable = setmetatable

local fatalPrintf = fatalPrintf

if not ____g_mir2x_inMainThread then
    fatalPrintf('setup tls outside main thread: %s', tostring(____g_mir2x_mainThreadId))
end

-- tls table list has default _ENV as _G for main thread
-- this requires current script should not sourced inside any function

local ____g_mir2x_threadLocalTableList = setmetatable({[____g_mir2x_mainThreadId] = _G}, {__mode = "k"})
local ____g_mir2x_threadLocalMetaTable = {}

function ____g_mir2x_threadLocalMetaTable:__index(k)
    local currThreadId, currInMainThread = coroutine.running()
    if currInMainThread then
        fatalPrintf('setup tls in main thread: %s', tostring(currThreadId))
    end

    local currThreadTable = ____g_mir2x_threadLocalTableList[currThreadId]
    if currThreadTable then
        if currThreadTable[k] == nil then
            return _G[k]
        else
            return currThreadTable[k]
        end
    else
        -- current thread doesn't have associated tls table allocated yet
        -- means there is no new variable allocated in the new thread, can allocate tls table here
        -- but current implement postpone the allocation and search _G
        return _G[k]
    end
end

function ____g_mir2x_threadLocalMetaTable:__newindex(k, v)
    local currThreadId, currInMainThread = coroutine.running()
    if currInMainThread then
        fatalPrintf('setup tls in main thread: %s', tostring(currThreadId))
    end

    local currThreadTable = ____g_mir2x_threadLocalTableList[currThreadId]
    if not currThreadTable then
        currThreadTable = setmetatable({_G = _G}, {__index = _G, __newindex = function(currThreadTable, key, value)
            -- when reaching here
            -- the thread tls table doesn't include key
            -- we check if _G includes it, if yes assin to _G.key, otherwise create new table entry in tls table
            if _G[key] == nil then
                rawset(currThreadTable, key, value)
            else
                _G[key] = value
            end
        end})

        ____g_mir2x_threadLocalTableList[currThreadId] = currThreadTable
    end
    currThreadTable[k] = v
end

function clearTLSTable()
    local currThreadId, currInMainThread = coroutine.running()
    if not currInMainThread then
        ____g_mir2x_threadLocalTableList[currThreadId]= nil
    end
end

-- convenient access to thread local variables via the `____g_mir2x_TLENV` table:
-- user can use ____g_mir2x_TLENV.var to explicitly declare a thread local varible and access it
____g_mir2x_TLENV = setmetatable({}, ____g_mir2x_threadLocalMetaTable)

-- change default lua env
-- makes all variables implicitly as thread-local-vars

-- this npchartlsconfig.lua get sourced as a chunk, but the _ENV is derived from main thread
-- so here chane it to ____g_mir2x_TLENV is good to replace _ENV for main(uid)
-- check this blog how upvalue/local/global works: https://luyuhuang.tech/2020/03/20/lua53-environment.html
_ENV = ____g_mir2x_TLENV

-- end tls setup
-- following function variable access always goes into tls first

--
-- )###"
