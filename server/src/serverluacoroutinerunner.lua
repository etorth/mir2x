--, u8R"###(
--

function _RSVD_NAME_luaCoroutineRunner_main(code, args)
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
    else
        fatalPrintf('Unknown error')
    end
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
            'integer'   -- monsterID
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

--
-- )###"
