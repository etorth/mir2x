function _RSVD_NAME_luaCoroutineRunner_codeMain(code, args)
    assertType(code, 'string')
    local func, err = load(code)
    if not func then
        fatalPrintf("Failed to load code: %s", err)
    else
        if args == nil then
            return func()
        else
            return func(table.unpack(args, 1, args.n))
        end
    end
end

-- wraps a function handed to runThread()
-- so the thread local storage of its coroutine is released the moment the coroutine leaves this call
-- which is what the string based spawn() gets for free from the do-block it generates around the script
-- without the wrapper the entry only disappears once the collector reaches the dead coroutine

function _RSVD_NAME_luaCoroutineRunner_funcMain(func)
    assertType(func, 'function')
    return function(...)
        local _RSVD_NAME_autoClear <close> = autoClearTLSTable()
        return func(...)
    end
end

function pause(msec)
    if msec == SYS_POSINF then
        while true do
            coroutine.yield()
        end
    end

    assertType(msec, 'integer')
    assert(msec >= 0)

    local oldTime = getTime()
    _RSVD_NAME_pauseYielding(msec)
    return getTime() - oldTime
end

-- close thread {key, seqID}, or every thread under key if seqID is nil or 0, returns true if any thread is found
-- a thread sitting in pause() is what a lua timer is, so this is how a timer gets cancelled
--
-- closing the calling thread itself never returns, the thread ends right there like exit()
-- it raises, and closes nothing, if called from a coroutine created in the thread, or where the thread can't yield
--
-- nor does closing a thread whose <close> handlers close the calling thread, the calling thread ends right after the close
-- it raises then, after the close, if called where the calling thread can't end
function closeThread(key, seqID)
    assertType(key, 'integer')
    assertType(seqID, 'integer', 'nil')

    local found, selfClose = _RSVD_NAME_closeThread(key, seqID or 0)
    if selfClose then
        -- resumeRunner() closes this thread at this yield, the loop is only a guard
        while true do
            coroutine.yield()
        end
    end

    _RSVD_NAME_endIfCloseRequested('closeThread()')
    return found
end

-- ends the calling thread here if it was asked to close while it ran, i.e. by a <close> handler of a thread it closed
-- a call that runs code of other threads, and goes on after them, ends with this, as runThread() does
-- raises if the thread can't end there, what names the call
function _RSVD_NAME_endIfCloseRequested(what)
    if _RSVD_NAME_closeRequested(what) then
        -- resumeRunner() closes this thread at this yield, the loop is only a guard
        while true do
            coroutine.yield()
        end
    end
end

-- run func on a new thread under key right away, returns when the new thread yields the first time or ends
-- never returns if the new thread closes the calling thread, i.e. switches the state of the calling quest state runner
-- raises if too many threads already run on top of each other on the C stack, i.e. state switches in a cycle with no yield
function runThread(key, func)
    assertType(key, 'integer')
    assertType(func, 'function')

    local seqID, closed = _RSVD_NAME_runThread(key, func)
    if closed then
        -- resumeRunner() closes this thread at this yield, the loop is only a guard
        while true do
            coroutine.yield()
        end
    end
    return key, seqID
end

-- closes the calling thread as closeThread() on itself does, then runs func on a new thread under key as runThread() does
-- func starts once the calling thread is closed, i.e. after its <close> handlers ran, from the code that resumed the calling thread
--
-- never returns, it raises and changes nothing if called from a coroutine created in the thread, or where the thread can't yield
function closeThreadThenRun(key, func)
    assertType(key, 'integer')
    assertType(func, 'function')

    _RSVD_NAME_closeThreadThenRun(key, func)

    -- resumeRunner() closes this thread at this yield, the loop is only a guard
    while true do
        coroutine.yield()
    end
end

function postNotify(addr, ...)
    assertType(addr, 'array')

    local uid = addr[1]
    local key = addr[2]
    local seq = addr[3]

    assertType(uid, 'integer')
    assertType(key, 'integer')
    assertType(seq, 'integer')

    assert(uid >  0)
    assert(key >  0)
    assert(seq >= 0)

    _RSVD_NAME_postNotify(uid, key, seq, table.pack(...))
end

function sendNotify(addr, ...)
    assertType(addr, 'array')

    local uid = addr[1]
    local key = addr[2]
    local seq = addr[3]

    assertType(uid, 'integer')
    assertType(key, 'integer')
    assertType(seq, 'integer')

    assert(uid >  0)
    assert(key >  0)
    assert(seq >= 0)

    if _RSVD_NAME_callFuncCoop('sendNotify', uid, key, seq, table.pack(...)) ~= SYS_EXECDONE then
        fatalPrintf('sendNotify failed')
    end
end

function pickNotify(count)
    if count == nil or count == SYS_POSINF then
        count = 0
    else
        assertType(count, 'integer')
        assert(count >= 0, 'count must be non-negative')
    end
    return _RSVD_NAME_pickNotify(count)
end

function waitNotify(timeout)
    assertType(timeout, 'integer', 'nil')
    timeout = argDefault(timeout, 0)
    assert(timeout >= 0, 'timeout must be non-negative')

    local result = _RSVD_NAME_waitNotify(timeout, threadKey, threadSeqID)
    if result then
        return table.unpack(result, 1, result.n)
    end

    coroutine.yield()

    local resList = pickNotify(1)
    if #resList == 1 then
        return table.unpack(resList[1], 1, resList[1].n)
    end

    -- timeout and not closed
    -- do not return nil since we support send nil
    --
    --      sendNotify(threadAddr, nil)
    --
    -- the tailing nil is also forwarded to waitNotify()
    return
end

function _RSVD_NAME_callFuncCoop(funcName, ...)
    local result = nil
    local function onDone(...)
        -- onDone()     -> {}
        -- onDone(nil)  -> {}
        -- onDone(1, 2) -> {1, 2}

        -- check if result is nil to determine if onDone is called
        -- because result shall not be nil in any case after onDone is called
        -- use pack(...) instead of {...} because returned sequence may contain nil's, especially tailing nil's
        result = table.pack(...)

        -- after this line
        -- C level will call resumeCORunner(threadKey) cooperatively
        -- this is black magic, we can not put resumeCORunner(threadKey) explicitly here in lua, see comments below:

        -- for callback function onDone
        -- it should keep simple as above, only setup marks/flags

        -- don't call other complext functions which gives callback hell
        -- I design the player runner in sequential manner

        -- more importantly, don't do runner-resume in the onDone callback
        -- which causes crash, because if we resume in onDone, then when the callback gets triggeerred, stack is:
        --
        --   --C-->onDone-->resumeCORunner(keyPair)-->code in this runner after _RSVD_NAME_requestSpaceMove/coroutine.yield()
        --     ^     ^            ^                   ^
        --     |     |            |                   |
        --     |     |            |                   +------ lua
        --     |     |            +-------------------------- C
        --     |     +--------------------------------------- lua
        --     +--------------------------------------------- C
        --
        -- see here C->lua->C->lua with yield/resume
        -- this crashes
    end

    -- NOTE
    -- when adding extra parameters to varidic arguments, it works if putting in front
    --
    --     f(extra_1, extra_2, ...)
    --
    -- but appending at end is bad, as following
    --
    --     f(..., extra_1, extra_2)
    --
    -- this way f() only gets the first argument in variadic argumeents

    local args = table.pack(...)

    args[args.n + 1] = onDone

    _G[string.format('_RSVD_NAME_%s%s', funcName, SYS_COOP)](table.unpack(args, 1, args.n + 1))

    -- onDone can get ran immedately in _RSVD_NAME_funcCoop
    -- in this situation we shall not yield

    -- TODO
    -- shall we use while-loop or single if-condition
    -- buggy code may call ServerLuaCoroutineRunner::resume() without call onDone

    while not result do
        coroutine.yield()
    end

    -- result gets assigned in onDone
    -- caller need to make sure in C side the return differs
    assertType(result, 'table')
    return table.unpack(result, 1, result.n)
end

function uidRemoteCall(uid, ...)
    local args = table.pack(...)

    assert(args.n >= 1)
    assertType(uid, 'integer')
    assertType(args[args.n], 'string')

    if uid == getUID() then
        fatalPrintf("Sending remote call to self is not allowed")
    end

    local resList = table.pack(_RSVD_NAME_callFuncCoop('remoteCall', uid, args[args.n], table.pack(table.unpack(args, 1, args.n - 1))))
    local resType = resList[1]

    if resType == SYS_EXECDONE then
        return table.unpack(resList, 2, resList.n)
    elseif resType == SYS_EXECBADUID then
        fatalPrintf('Invalid uid: %d', uid)
    elseif resType == SYS_EXECERROR then
        fatalPrintf('Remote call to %s failed: %s', getUIDString(uid), resList[2])
    else
        fatalPrintf('Unknown error')
    end
end

-- example of itemCfgList:
--
-- itemCfgList =
-- {
--     {
--         item = '屠龙',
--         odds = 1
--     },
--
--     {
--         item =
--         {
--             itemID = '强效太阳水',
--             seqID = 0,
--             count = 1
--         },
--         odds = 1
--     }
-- }
function setMonsterDropOnDie(monsterUID, itemCfgList, opts)
    assertType(monsterUID, 'integer')
    assert(isMonster(monsterUID))

    assertType(itemCfgList, 'array', 'nil')
    assertType(opts, 'table', 'nil')

    -- by default don't allow default drop
    -- so setMonsterDropOnDie(UID) makes the monster drop nothing

    local playerUID = 0
    local allowDefaultDrop = false

    if opts then
        if opts.player ~= nil then
            playerUID = opts.player
            assertType(playerUID, 'integer')

            if playerUID ~= 0 then
                assert(isPlayer(playerUID))
            end
        end

        if opts.defaultDrop ~= nil then
            allowDefaultDrop = opts.defaultDrop
            assertType(allowDefaultDrop, 'boolean')
        end
    end

    return _RSVD_NAME_callFuncCoop('setMonsterDropOnDie', monsterUID, playerUID, allowDefaultDrop, itemCfgList or {})
end

local _RSVD_NAME_triggerConfigList = {
    -- trigger parameter config
    -- [1] : trigger type in string
    -- [2] : trigger parameter types
    -- [3] : trigger parameter extra checking, optional

    [SYS_ON_LEVELUP] = {
        'SYS_ON_LEVEL',
        {
            'integer',  -- oldLevel
            'integer'   -- newLevel
        },

        function(args)
            assert(args[1] < args[2])
        end
    },

    [SYS_ON_KILL] = {
        'SYS_ON_KILL',
        {
            'integer'   -- monsterUID
        },
    },

    [SYS_ON_GAINEXP] = {
        'SYS_ON_GAINEXP',
        {
            'integer'   -- exp
        },
    },

    [SYS_ON_GAINGOLD] = {
        'SYS_ON_GAINGOLD',
        {
            'integer'   -- gold
        },
    },

    [SYS_ON_ONLINE] = {
        'SYS_ON_ONLINE',
        {
        },
    },

    [SYS_ON_OFFLINE] = {
        'SYS_ON_OFFLINE',
        {
        },
    },

    [SYS_ON_DIE] = {
        'SYS_ON_DIE',
        {
        },
    },

    [SYS_ON_REVIVE] = {
        'SYS_ON_REVIVE',
        {
        },
    },

    [SYS_ON_GAINITEM] = {
        'SYS_ON_GAINITEM',
        {
            'integer'   -- itemID
        },
    },

    [SYS_ON_APPEAR] = {
        'SYS_ON_APPEAR',
        {
            'integer'   -- uid
        },
    },
}

function _RSVD_NAME_triggerConfig(triggerType)
    return _RSVD_NAME_triggerConfigList[triggerType]
end

if SYS_DEBUG then
    -- test asInitString

    local ins = require '3rdparty.inspect'

    u = {["2"]=2,[3]=3,["4\"4[[44]]''"]={4,['55\'5']={[6]=7}}}
    load(string.format([[v=%s]],asInitString(u)))()

    assert(ins.inspect(u) == ins.inspect(v))
    u = nil
    v = nil

    u = randString(10000, [[='"{}[]()]])
    load(string.format([[v=%s]],asInitString(u)))()

    assert(u == v)
    u = nil
    v = nil
end

server = {}

server.utils  = require 'api.utils'
server.player = require 'api.player'
server.quest  = require 'api.quest'
server.npc    = require 'api.npc'
