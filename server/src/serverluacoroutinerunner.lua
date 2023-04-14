--, u8R"###(
--

function _RSVD_NAME_luaCoroutineRunner_main(code)
    assertType(code, 'string')
    return (load(code))()
end

function sendNotify(...)
    return _RSVD_NAME_callFuncCoop('sendNotify', ...)
end

function waitNotify(count)
    assertType(count, 'integer', 'nil')

    count = argDefault(count, 0)
    assert(count >= 0, 'count must be non-negative')

    local resList = _RSVD_NAME_waitNotify(getTLSTable().threadKey, getTLSTable().threadSeqID)
    assertType(resList, 'table')

    if count == 0 then
        return resList
    end

    while #resList < count do
        coroutine.yield()
        local newResList = _RSVD_NAME_waitNotify(getTLSTable().threadKey, getTLSTable().threadSeqID)

        assertType(newResList, 'table')
        for k, v in pairs(newResList) do
            table.insert(resList, v)
        end
    end

    return resList
end

function _RSVD_NAME_callFuncCoop(funcName, ...)
    local result = nil
    local function onDone(...)
        -- onDone()     -> {}
        -- onDone(nil)  -> {}
        -- onDone(1, 2) -> {1, 2}

        -- check if result is nil to determine if onDone is called
        -- because result shall not be nil in any case after onDone is called
        result = {...}

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
        --   --C-->onDone-->resumeCORunner(getTLSTable().threadKey)-->code in this runner after _RSVD_NAME_requestSpaceMove/coroutine.yield()
        --     ^     ^            ^                                   ^
        --     |     |            |                                   |
        --     |     |            |                                   +------ lua
        --     |     |            +------------------------------------------ C
        --     |     +------------------------------------------------------- lua
        --     +------------------------------------------------------------- C
        --
        -- see here C->lua->C->lua with yield/resume
        -- this crashes
    end

    local args = {...}

    table.insert(args, onDone)
    table.insert(args, getTLSTable().threadKey)
    table.insert(args, getTLSTable().threadSeqID)

    _G[string.format('_RSVD_NAME_%s%s', funcName, SYS_COOP)](table.unpack(args))

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
    return table.unpack(result)
end

function uidExecute(uid, code, ...)
    local resList = {_RSVD_NAME_callFuncCoop('uidExecute', uid, code:format(...))}
    local resType = resList[1]

    if resType == SYS_EXECDONE then
        return table.unpack(resList, 2)
    elseif resType == SYS_BADUID then
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

    [SYS_ON_GAINITEM] = {
        'SYS_ON_GAINITEM',
        {
            'integer'   -- itemID
        },
    },
}

function _RSVD_NAME_triggerConfig(triggerType)
    return _RSVD_NAME_triggerConfigList[triggerType]
end

--
-- )###"
