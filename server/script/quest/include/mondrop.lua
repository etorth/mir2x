-- quest rewards that come off a monster's corpse
--
-- legacy Envir kept these in QuestDiary/NQ_*/MonQuest/Nm_*.txt: a script run when a
-- monster dies, gated on the killer's quest flags, that hands over a quest item and
-- moves the quest along
--
-- the trigger lives on the quest actor instead of on the player, so it keeps working
-- after a relog, and it reads the player's persisted quest state on every kill

local mondrop = {}

-- legacy ramp, see Nm_Oma.txt and friends
--
-- the counter jumps straight to 3 on the first kill and climbs by one after that, and the
-- last two steps of a long ramp are a coin flip each so the drop doesn't land on a
-- predictable kill, a short ramp climbs straight up
local function bumpKillCount(count, kills)
    if count < 3 then
        return 3
    end

    if kills >= 5 and count > kills - 2 and math.random(2) ~= 1 then
        return count
    end
    return count + 1
end

-- accept 'name', {'name', count}, or a list of either
local function asItemList(arg)
    if arg == nil then
        return {}
    end

    if type(arg) == 'string' then
        return {{arg, 1}}
    end

    assertType(arg, 'table')
    if type(arg[1]) == 'string' and (arg[2] == nil or math.type(arg[2]) == 'integer') then
        return {{arg[1], arg[2] or 1}}
    end

    local result = {}
    for _, v in ipairs(arg) do
        for _, item in ipairs(asItemList(v)) do
            table.insert(result, item)
        end
    end
    return result
end

local function asNameList(arg)
    assertType(arg, 'string', 'table')
    if type(arg) == 'string' then
        return {arg}
    end

    for _, v in ipairs(arg) do
        assertType(v, 'string')
    end
    return arg
end

local function inState(playerUID, stateList)
    local state = dbGetQuestState(playerUID)
    if state == nil then
        return false
    end

    for _, v in ipairs(stateList) do
        if v == state then
            return true
        end
    end
    return false
end

-- returns true when the drop fired, so a monster carrying more than one drop only ever
-- hands over the first one that is live
local function runDrop(playerUID, drop)
    if not inState(playerUID, drop.state) then
        return false
    end

    for _, item in ipairs(drop.need) do
        if not server.player.hasItem(playerUID, item[1], item[2]) then
            return false
        end
    end

    -- can't hand out a second copy of something the player is still carrying
    if drop.once then
        for _, item in ipairs(drop.give) do
            if server.player.hasItem(playerUID, item[1], item[2]) then
                return false
            end
        end
    end

    if drop.chance > 1 and math.random(drop.chance) ~= 1 then
        return false
    end

    if drop.kills > 1 then
        local count = bumpKillCount(dbGetQuestVar(playerUID, drop.counter) or 0, drop.kills)
        if count <= drop.kills then
            dbSetQuestVar(playerUID, drop.counter, count)
            return false
        end
        dbSetQuestVar(playerUID, drop.counter, nil)
    end

    for _, item in ipairs(drop.take) do
        server.player.removeItem(playerUID, item[1], item[2])
    end

    for _, item in ipairs(drop.give) do
        server.player.addItem(playerUID, item[1], item[2])
    end

    if drop.say then
        server.player.postString(playerUID, drop.say)
    end

    if drop.moveTo then
        server.player.spaceMove(playerUID, table.unpack(drop.moveTo))
    end

    -- last, it can clear the state this drop is gated on
    if drop.setState then
        setQuestState{uid = playerUID, state = drop.setState}
    end
    return true
end

-- call after setQuestFSMTable, the state names get validated against the FSM
--
--     mondrop.setDropOnKill
--     {
--         {
--             monster  = '千年毒蛇',          -- name, or list of names
--             state    = 'quest_find_gall',  -- quest states this drop is live in
--             kills    = 10,                 -- optional, roughly how many kills it takes
--             chance   = 2,                  -- optional, 1/chance per kill instead
--             once     = true,               -- optional, don't hand out a second copy
--             need     = '角笛',              -- optional, must be carrying this
--             give     = '千年毒蛇胆汁',       -- optional, 'name' / {'name', count} / list
--             take     = '角笛',              -- optional, same shapes as give
--             setState = 'quest_got_gall',   -- optional, state to move to afterwards
--             moveTo   = {'D001', 303, 70},  -- optional, where to put the player
--             say      = '...',              -- optional, message to the player
--         },
--     }
--
function mondrop.setDropOnKill(dropList)
    assertType(dropList, 'table')

    -- index by monster id, a kill then only walks the drops that could fire
    local dropListByMonster = {}

    for _, drop in ipairs(dropList) do
        assertType(drop, 'table')
        assertType(drop.kills, 'integer', 'nil')
        assertType(drop.chance, 'integer', 'nil')
        assertType(drop.once, 'boolean', 'nil')
        assertType(drop.say, 'string', 'nil')
        assertType(drop.moveTo, 'table', 'nil')
        assertType(drop.setState, 'string', 'nil')

        if drop.kills and drop.chance then
            fatalPrintf('Monster drop takes kills or chance, not both')
        end

        local monsterNameList = asNameList(drop.monster)

        local parsed =
        {
            state    = asNameList(drop.state),
            need     = asItemList(drop.need),
            give     = asItemList(drop.give),
            take     = asItemList(drop.take),
            kills    = drop.kills or 1,
            chance   = drop.chance or 1,
            once     = drop.once,
            say      = drop.say,
            moveTo   = drop.moveTo,
            setState = drop.setState,

            -- keyed on the monsters so the count survives a restart, legacy used one
            -- counter per monster script
            counter  = drop.counter or ('mondrop_' .. table.concat(monsterNameList, '_')),
        }

        for _, state in ipairs(parsed.state) do
            if not hasQuestState(state) then
                fatalPrintf('Monster drop gated on unknown quest state %s', state)
            end
        end

        if parsed.setState and (parsed.setState ~= SYS_DONE) and (not hasQuestState(parsed.setState)) then
            fatalPrintf('Monster drop moves to unknown quest state %s', parsed.setState)
        end

        for _, list in ipairs({parsed.need, parsed.give, parsed.take}) do
            for _, item in ipairs(list) do
                if getItemID(item[1]) <= 0 then
                    fatalPrintf('Monster drop refers to unknown item %s', item[1])
                end
            end
        end

        for _, monsterName in ipairs(monsterNameList) do
            local monsterID = getMonsterID(monsterName)
            if monsterID <= 0 then
                fatalPrintf('Monster drop refers to unknown monster %s', monsterName)
            end

            if not dropListByMonster[monsterID] then
                dropListByMonster[monsterID] = {}
            end
            table.insert(dropListByMonster[monsterID], parsed)
        end
    end

    addQuestTrigger(SYS_ON_KILL, function(playerUID, monsterID)
        for _, drop in ipairs(dropListByMonster[monsterID] or {}) do
            if runDrop(playerUID, drop) then
                return
            end
        end
    end)
end

return mondrop
