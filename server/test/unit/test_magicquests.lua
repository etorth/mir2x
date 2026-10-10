local root = (arg[1] and (arg[1] .. '/') or '') .. 'server/script/quest/'
local names = {
    '基本剑术任务', '攻杀剑术任务', '刺杀剑术任务', '野蛮冲撞任务', '火球术任务',
    '抗拒火环任务', '诱惑之光任务', '雷电术任务', '瞬息移动任务', '大火球任务',
    '地狱火任务', '爆裂火焰任务', '疾光电影任务',
}
local function harness(name)
    local h = {state = nil, vars = {}, runtime = {}, handlers = {}, triggers = {}, items = {},
               monsterMap = 500, playerMap = 500, mapName = '沙漠_42', nextID = 100,
               threads = {}, closed = {}, spawned = {}, drops = {}, moves = {}, job = '法师'}
    local env = setmetatable({}, {__index = _G})
    env._G = env
    for _, k in ipairs({'SYS_ENTER', 'SYS_DONE', 'SYS_ON_KILL', 'SYS_ON_GAINITEM', 'SYS_ON_ONLINE',
                       'SYS_ON_OFFLINE', 'SYS_ON_DIE', 'SYS_LABEL', 'SYS_CHECKACTIVE', 'SYS_EXIT',
                       'SYS_EPUID', 'SYS_EPQST'}) do env[k] = k end
    env.assertType = function(v) return v end
    env.fatalPrintf = function(s) error(s) end
    local ids, labels = {}, {}
    local function id(s)
        if type(s) == 'number' then return s end
        if not ids[s] then h.nextID = h.nextID + 1; ids[s] = h.nextID; labels[h.nextID] = s end
        return ids[s]
    end
    env.getItemID, env.getMapID, env.getMonsterID = id, id, id
    env.getMonsterName = function(v) return labels[v] end
    env.getUID = function() return 99 end
    env.getQuestName = function() return name end
    env.getNPCharUID = function() return 88 end
    env.dbGetQuestState = function() return h.state end
    env.dbGetQuestVar = function(_, k) return h.vars[k] end
    env.dbSetQuestVar = function(_, k, v) h.vars[k] = v end
    env.getQuestRuntimeVar = function(_, k) return h.runtime[k] end
    env.setQuestRuntimeVar = function(_, k, v) h.runtime[k] = v end
    env.setQuestDesp = function() end
    env.hasQuestState = function() return true end
    env.asInitString = function() return '{}' end
    env.addQuestTrigger = function(k, f) h.triggers[k] = f end
    env.setQuestFSMTable = function(t) h.fsm = t end
    env.setQuestHandler = function(_, t) table.insert(h.handlers, t) end
    env.runQuestThread = function(f) h.nextID = h.nextID + 1; h.threads[h.nextID] = f; return h.nextID end
    env.pause = function() end
    env.closeThread = function(t) h.closed[t] = true end
    env.closeInstanceMap = function() end
    env.loadInstanceMap = function() h.nextID = h.nextID + 1; return h.nextID + 1000 end
    env.loadBaseMap = function() return 1 end
    env.clearMonster = function() end
    env.addTrigger = function() h.nextID = h.nextID + 1; return h.nextID end
    env.deleteTrigger = function() end
    env.isMonster = function() return true end
    -- a monster uid whose getMonsterName() is its name, as a real uid carries its monster id
    h.spawn = function(n)
        h.nextID = h.nextID + 1
        labels[h.nextID] = n
        return h.nextID
    end
    env.addMonster = function(n, x, y)
        local monsterUID = h.spawn(n)
        table.insert(h.spawned, {n, x, y, uid = monsterUID, map = h.remoteTarget})
        return monsterUID
    end
    env.setMonsterDropOnDie = function(uid, items, opts)
        table.insert(h.drops, {uid = uid, items = items, opts = opts})
        return true
    end
    env.server = {player = {}, quest = {}}
    local p = env.server.player
    p.hasItem = function(_, n, count) return (h.items[n] or 0) >= count end
    p.addItem = function(_, n, count) h.items[n] = (h.items[n] or 0) + count end
    p.removeItem = function(_, n, count) h.items[n] = (h.items[n] or 0) - count end
    p.hasJob = function(_, n) return h.job == n end
    p.hasMagic = function() return h.magic or false end
    p.getLevel = function() return 50 end
    p.getGender = function() return true end
    p.getMapName = function() return h.mapName end
    p.postString = function() end
    p.deliverGold = function() end
    p.spaceMove = function(_, map) h.playerMap = map; table.insert(h.moves, map) end
    env.server.quest.getState = function() return h.state end
    env.setQuestState = function(t)
        if t.from and t.from ~= h.state then return false end
        h.state = t.state
        if h.fsm and h.fsm[h.state] then h.fsm[h.state](1, {}) end
        return true
    end
    env.server.quest.setState = function(_, t) return env.setQuestState(t) end
    env.uidRemoteCall = function(uid, ...)
        local a = table.pack(...)
        local code = a[a.n]
        assert(type(code) == 'string')
        local remote = setmetatable({getMapUID = function()
            -- a monster has no lua runner, a remote call to one stops the server
            assert(uid == 1, 'remote call to a monster')
            return h.playerMap
        end}, {__index = env})
        local saved = h.remoteTarget
        h.remoteTarget = uid
        local result = table.pack(assert(load(code, 'remote', 't', remote))(table.unpack(a, 1, a.n - 1)))
        h.remoteTarget = saved
        return table.unpack(result, 1, result.n)
    end
    env.setupNPCQuestBehavior = function(_, _, uid, init, code)
        assert(load(code, 'npc callback', 't', env))
    end
    local module
    env.require = function(n)
        if n == 'quest.include.mondrop' then
            if not module then module = assert(loadfile(root .. 'include/mondrop.lua', 't', env))() end
            return module
        elseif n == 'include.dialog' then
            return {post = function() end, link = function() return {} end}
        end
        return require(n)
    end
    assert(loadfile(root .. name .. '.lua', 't', env))()
    h.env, h.module = env, env.require('quest.include.mondrop')
    return h
end
for _, name in ipairs(names) do harness(name) end
for _, name in ipairs({'抗拒火环任务', '瞬息移动任务', '大火球任务', '疾光电影任务'}) do
    local h = harness(name)
    local entry = h.handlers[#h.handlers]
    h.job = '战士'; assert(not entry.SYS_CHECKACTIVE(1), name)
    h.job = '道士'; assert(not entry.SYS_CHECKACTIVE(1), name)
    h.job = '法师'; assert(entry.SYS_CHECKACTIVE(1), name)
end
do
    local h = harness('野蛮冲撞任务')
    h.job = '战士'; h.magic = true
    for i = 2, #h.handlers do assert(not h.handlers[i].SYS_CHECKACTIVE(1)) end
end
for _, spec in ipairs({{10, 10}, {4, 4}, {3, 3}}) do
    local h = harness('刺杀剑术任务')
    h.state = 'active'
    local call = h.module.addDropTrigger(1, {{monster = '测试', counterMode = {threshold = spec[1], initial = 3}, give = '奖励'}})
    for kill = 1, spec[2] do
        h.module._runDropOnKill(1, call, h.env.getMonsterID('测试'))
        assert((h.items['奖励'] or 0) == (kill == spec[2] and 1 or 0), 'counter boundary')
    end
end
do
    local h = harness('刺杀剑术任务')
    h.state = 'active'
    local drop = {monster = '测试', counterMode = {threshold = 3, initial = 3, completed = 5},
                  once = true, give = '骨头'}
    local call = h.module.addDropTrigger(1, {drop})
    for _ = 1, 3 do h.module._runDropOnKill(1, call, h.env.getMonsterID('测试')) end
    assert(h.items['骨头'] == 1 and h.vars['mondrop_测试'] == 5, 'source completion counter latched')
    h.items['骨头'] = nil
    call = h.module.addDropTrigger(1, {drop})
    for _ = 1, 10 do h.module._runDropOnKill(1, call, h.env.getMonsterID('测试')) end
    assert(not h.items['骨头'] and h.vars['mondrop_测试'] == 5, 'lost item and reinstalled trigger cannot drop again')
end
for _, mode in ipairs({
    {threshold = 10, initial = 3, randomFrom = 9, incrementChance = 2},
    {threshold = 5, initial = 3, randomFrom = 4, incrementChance = 2},
    {threshold = 12, initial = 3, randomFrom = 11, incrementChance = 2},
}) do
    local h = harness('刺杀剑术任务')
    local roll, rolls = 2, 0
    h.env.math = setmetatable({random = function(n)
        assert(n == 2); rolls = rolls + 1; return roll
    end}, {__index = math})
    h.state = 'active'
    local call = h.module.addDropTrigger(1, {{monster = '测试', counterMode = mode, give = '奖励'}})
    local kill = function() h.module._runDropOnKill(1, call, h.env.getMonsterID('测试')) end
    while (h.vars['mondrop_测试'] or 0) < mode.randomFrom do kill() end
    assert(rolls == 0, 'early ramp deterministic')
    kill()
    assert(h.vars['mondrop_测试'] == mode.randomFrom and rolls == 1, 'random ramp can stall')
    roll = 1
    while (h.vars['mondrop_测试'] or 0) <= mode.threshold do kill() end
    assert(not h.items['奖励'], 'threshold exceeded, reward waits until next kill')
    kill()
    assert(h.items['奖励'] == 1)
end
do
    local h = harness('刺杀剑术任务')
    h.state = 'active'
    local call = h.module.addDropTrigger(1, {{monster = '测试', kills = 3, give = '奖励'}})
    h.module._runDropOnKill(1, call, h.env.getMonsterID('测试'))
    assert(not h.items['奖励'])
    h.module._runDropOnKill(1, call, h.env.getMonsterID('测试'))
    assert(h.items['奖励'] == 1, 'legacy behavior unchanged')
end
do
    local h = harness('野蛮冲撞任务')
    local captured
    h.module.addDropTrigger = function(_, list) captured = list end
    h.state = 'quest_find_stones'
    h.fsm[h.state](1, {})
    assert(#captured == 5)
    for count = 0, 5 do
        h.items['诺玛石'] = count
        local eligible = 0
        for _, d in ipairs(captured) do
            local need = not d.need or h.env.server.player.hasItem(1, d.need[1], d.need[2])
            local excluded = h.env.server.player.hasItem(1, d.exclude[1], d.exclude[2])
            if need and not excluded then eligible = eligible + 1; assert(d.chance == 2) end
            assert(#d.map == 6)
        end
        assert(eligible == (count < 5 and 1 or 0), 'exactly one stone roll')
    end
    h.state = 'quest_found_stones'; h.items['诺玛石'] = 0; h.fsm[h.state](1, {})
    assert(#captured == 1 and not captured[1].give, 'fifth stone flag persists')
end
for count = 0, 5 do
    for roll = 1, 2 do
        local h = harness('野蛮冲撞任务')
        local rolls = 0
        h.env.math = setmetatable({random = function(n)
            assert(n == 2)
            rolls = rolls + 1
            return roll
        end}, {__index = math})
        h.items['诺玛石'] = count
        h.env.setQuestState{uid = 1, state = 'quest_find_stones'}
        h.module._runDropOnKill(1, 1, h.env.getMonsterID('诺玛法老'))
        assert(rolls == (count < 5 and 1 or 0), 'one roll, not overlapping branches')
        assert(h.items['诺玛石'] == count + ((count < 5 and roll == 1) and 1 or 0))
        if count == 4 and roll == 1 then
            assert(h.state == 'quest_found_stones')
            h.module._runDropOnKill(1, 2, h.env.getMonsterID('诺玛法老'))
            assert(h.items['诺玛石'] == 5, 'never sixth stone')
        end
    end
end
-- the uids of the monsters named name spawned on mapUID, in spawn order
local function spawnedOn(h, mapUID, name)
    local uidList = {}
    for _, s in ipairs(h.spawned) do
        if s.map == mapUID and s[1] == name then table.insert(uidList, s.uid) end
    end
    return uidList
end
local function blockers(h, index, side)
    return spawnedOn(h, h.runtime['forkMapUID' .. index], (side == 'L') and h.env.leftMonster or h.env.rightMonster)
end
do
    local h = harness('瞬息移动任务')
    h.state = 'quest_in_trial'
    local right, left, foreign = h.spawn(h.env.rightMonster), h.spawn(h.env.leftMonster), h.spawn(h.env.rightMonster)
    h.runtime.forkBlockers5 = {[right] = true, [left] = true}
    h.vars.forkPath = {'L', 'L', 'R', 'L'}
    h.triggers.SYS_ON_KILL(1, foreign)
    assert(#h.vars.forkPath == 4, 'a blocker of no fork of this run ignored')
    h.triggers.SYS_ON_KILL(1, right)
    assert(h.state == 'quest_in_trial' and h.vars.forkPath[5] == 'R', 'first final kill only records')
    h.triggers.SYS_ON_KILL(1, left)
    assert(h.state == 'quest_trial_passed', 'second final kill passes')
end
for route = 1, 2 do
    local h = harness('瞬息移动任务')
    h.env.setQuestState{uid = 1, state = 'quest_enter_trial'}
    assert(h.state == 'quest_in_trial' and h.runtime.trialTimer)
    -- @mugong_fly_next8_5 and next11_4
    assert(#spawnedOn(h, h.runtime.forkMapUID1, '骷髅战士') == 2, 'first stock of fork 1')
    assert(#spawnedOn(h, h.runtime.forkMapUID4, '沃玛勇士') == 8, 'first stock of fork 4')
    for index, side in ipairs(h.env.goodPaths[route]) do
        h.triggers.SYS_ON_KILL(1, blockers(h, index, side)[1])
        assert(h.state == 'quest_in_trial')
        if index < 5 then assert(h.moves[#h.moves] == h.runtime['forkMapUID' .. (index + 1)]) end
    end
    h.triggers.SYS_ON_KILL(1, blockers(h, 5, 'R')[2])
    assert(h.state == 'quest_trial_passed')
end
do
    -- one spell takes both blockers of a fork: the second kill comes in after the move to the next fork
    local h = harness('瞬息移动任务')
    h.env.setQuestState{uid = 1, state = 'quest_enter_trial'}
    h.triggers.SYS_ON_KILL(1, blockers(h, 1, 'L')[1])
    h.triggers.SYS_ON_KILL(1, blockers(h, 1, 'R')[1])
    assert(#h.vars.forkPath == 1 and h.vars.forkPath[1] == 'L', 'the other blocker of a fork already taken counts for nothing')
end
do
    local h = harness('瞬息移动任务')
    h.env.setQuestState{uid = 1, state = 'quest_enter_trial'}
    local timer = h.runtime.trialTimer
    h.vars.forkPath = {'R', 'R', 'R', 'R', 'R'}
    local count1, count4 = #spawnedOn(h, h.runtime.forkMapUID1, '骷髅战士'), #spawnedOn(h, h.runtime.forkMapUID4, '沃玛勇士')
    h.triggers.SYS_ON_KILL(1, blockers(h, 5, 'R')[1])
    assert(h.state == 'quest_in_trial' and h.vars.forkPath == nil)
    assert(h.closed[timer] and h.runtime.trialTimer ~= timer, 'wrong path renews timer')
    assert(h.moves[#h.moves] == h.runtime.forkMapUID1)
    -- @mugong_fly_restart1_5 and restart4_4
    assert(#spawnedOn(h, h.runtime.forkMapUID1, '骷髅战士') - count1 == 1, 'restock of fork 1')
    assert(#spawnedOn(h, h.runtime.forkMapUID4, '沃玛勇士') - count4 == 7, 'restock of fork 4')
    local old = blockers(h, 1, 'L')[1]
    h.triggers.SYS_ON_KILL(1, old)
    assert(h.vars.forkPath == nil, 'a blocker of the run before the restock counts for nothing')
end
for _, name in ipairs({'雷电术任务', '地狱火任务'}) do
    local h = harness(name)
    h.state = 'quest_in_trial'; h.runtime.trialMapUID = 500; h.playerMap = 900
    h.triggers.SYS_ON_KILL(1, h.env.getMonsterID(h.env.bossName or h.env.markedMonster))
    assert(h.state == 'quest_in_trial', 'foreign trial kill ignored')
end
do
    local h = harness('雷电术任务')
    h.state = 'quest_in_trial'; h.runtime.trialMapUID = 500; h.playerMap = 500
    h.triggers.SYS_ON_KILL(1, h.env.getMonsterID(h.env.bossName))
    assert(h.state == 'quest_trial_passed', 'boss killed in the trial passes')
end
do
    local h = harness('地狱火任务')
    h.state = 'quest_enter_trial'; h.fsm[h.state](1, {})
    assert(#h.drops == 1 and h.drops[1].items[1].item == '新火镜')
    -- MonItems/火焰沃玛62.txt: the book only, no default drop, on the ground
    assert(h.drops[1].opts == nil, 'book drops on ground, nothing else')
    h.playerMap = h.runtime.trialMapUID
    h.triggers.SYS_ON_KILL(1, h.env.getMonsterID(h.env.markedMonster))
    assert(h.state == 'quest_in_trial' and not h.items['新火镜'], 'kill does not grant book or pass')
    h.playerMap = 900
    h.triggers.SYS_ON_GAINITEM(1, h.env.getItemID('新火镜'))
    assert(h.state == 'quest_in_trial')
    h.playerMap = h.runtime.trialMapUID; h.items['新火镜'] = 1
    h.triggers.SYS_ON_GAINITEM(1, h.env.getItemID('新火镜'))
    assert(h.state == 'quest_trial_passed', 'pickup passes')
end
print('magic fidelity: all 13 scripts, gates, ramps, stone branches, maps, timer, and pickup passed')
