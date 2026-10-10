local root = arg[1] or '.'

local function fixture(name)
    local f = {
        handlers = {}, triggers = {}, behaviors = {}, runtime = {}, monsterMaps = {},
        monsterCounts = {}, items = {}, flags = {}, added = {}, closed = {}, posts = {},
        state = nil, job = '道士', level = 99, gold = 0, countQueries = 0,
    }
    local env = setmetatable({}, {__index = _G})
    env._G = env
    for _, key in ipairs({
        'SYS_ENTER', 'SYS_DONE', 'SYS_EXIT', 'SYS_LUANIL', 'SYS_CHECKACTIVE',
        'SYS_LABEL', 'SYS_ON_KILL', 'SYS_ON_ONLINE', 'SYS_ON_OFFLINE', 'SYS_ON_DIE',
        'SYS_EPUID', 'SYS_EPQST',
    }) do
        env[key] = key
    end
    env.getUID = function() return 100 end
    env.getQuestName = function() return name end
    env.getNPCharUID = function(map, npc) return map .. '/' .. npc end
    env.asInitString = function(value) return string.format('%q', value) end
    env.setQuestFSMTable = function(states) f.states = states end
    env.getQuestRuntimeVar = function(uid, key) return f.runtime[key] end
    env.setQuestRuntimeVar = function(uid, key, value) f.runtime[key] = value end
    env.dbGetQuestState = function() return f.state end
    env.setQuestState = function(opts)
        if opts.from and opts.from ~= f.state then return false end
        f.state = opts.state
        if opts.state == 'SYS_DONE' then f.runtime = {} end
        return true
    end
    env.closeInstanceMap = function(uid) f.closed[#f.closed + 1] = uid end
    env.isMonster = function(uid, monster) return f.monsterName == monster end
    env.addQuestTrigger = function(event, callback)
        f.triggers[event] = f.triggers[event] or {}
        table.insert(f.triggers[event], callback)
    end
    env.setQuestHandler = function(quest, handlers) f.handlers[f.remoteTarget] = handlers end
    env.setupNPCQuestBehavior = function(map, npc, uid, args, code)
        f.behaviors[env.getNPCharUID(map, npc)] = {args = args, code = code}
    end
    env.setupInstanceNPCBehavior = function() end
    env.setupInstanceUIDGridTrigger = function() end
    env.setupMapUIDGridTrigger = function() end
    env.setupMapGridTrigger = function() end
    env.setQuestDesp = function() end
    env.hasQuestFlag = function(uid, key) return f.flags[key] == true end
    env.addQuestFlag = function(uid, key) f.flags[key] = true end

    local player = {}
    player.hasJob = function(uid, job) return f.job == job end
    player.getLevel = function() return f.level end
    player.hasMagic = function() return false end
    player.hasItem = function(uid, item, count) return (f.items[item] or 0) >= count end
    player.removeItem = function(uid, item, count)
        assert(player.hasItem(uid, item, count), 'Removing unavailable item')
        f.items[item] = f.items[item] - count
        return true
    end
    player.addItem = function(uid, item, count)
        f.items[item] = (f.items[item] or 0) + count
        f.added[#f.added + 1] = {item, count}
    end
    player.deliverGold = function(uid, amount) f.gold = f.gold + amount end
    player.postString = function() end
    player.spaceMove = function() end
    env.server = {player = player, quest = {
        getState = function() return f.state end,
        setState = function(uid, opts) return env.setQuestState(opts) end,
    }}
    local dialog = {
        post = function(uid, path, text, links) f.posts[#f.posts + 1] = {text = text, links = links} end,
        link = function(id, label, opts) return {id = id, label = label, opts = opts} end,
    }
    env.require = function(module)
        if module == 'include.dialog' then return dialog end
        if module == 'quest.include.mondrop' then return {addDropTrigger = function() end} end
        error('Unexpected module: ' .. module)
    end
    env.uidRemoteCall = function(target, ...)
        local args = table.pack(...)
        local code = args[args.n]
        if code:find('getMapUID', 1, true) then
            assert(target == 1, 'a monster takes no remote call, the kill scope comes from the killer')
            return f.playerMap
        end
        if f.monsterCounts[target] ~= nil then
            assert(code:find('getMonsterCount', 1, true), 'Unexpected map query')
            f.countQueries = f.countQueries + 1
            return f.monsterCounts[target]
        end
        f.remoteTarget = target
        return assert(load(code, 'remote call', 't', env))(table.unpack(args, 1, args.n - 1))
    end
    assert(loadfile(root .. '/server/script/quest/' .. name .. '.lua', 't', env))()
    f.env = env
    return f
end

local teacher = '本馆_1_002/清明子_1'
local gateCases = {
    {'治愈术任务', teacher},
    {'精神力战法任务', teacher},
    {'施毒术任务', teacher},
    {'召唤骷髅任务', teacher},
    {'隐身术任务', teacher},
    {'集体隐身术任务', '比奇县_0/杂货商_1'},
    {'幽灵盾任务', teacher},
    {'神圣战甲术任务', teacher},
    {'困魔咒任务', '道馆_1/大悲善僧_1'},
    {'群体治愈术任务', '道馆_1/大悲善僧_1'},
}
for _, case in ipairs(gateCases) do
    local f = fixture(case[1])
    local handlers = assert(f.handlers[case[2]], case[1] .. ': missing menu')
    for _, job in ipairs({'战士', '法师', '道士'}) do
        f.job = job
        assert(handlers.SYS_CHECKACTIVE(1) == (job == '道士'),
            case[1] .. ': incorrect job eligibility for ' .. job)
        if case[3] and job ~= '道士' then
            f.posts = {}
            handlers.SYS_ENTER(1)
            assert(#f.posts == 1 and type(f.posts[1].text) == 'string'
                and f.posts[1].text:find('只有道士', 1, true),
                case[1] .. ': direct wrong-job entry did not reject')
            assert(f.state == nil and #f.added == 0, case[1] .. ': wrong-job entry changed quest')
        end
    end
end

for _, case in ipairs({
    {'精神力战法任务', 'quest_in_trial', 'trialMapUID', '半兽战士', 'quest_trial_passed'},
    {'召唤骷髅任务', 'quest_in_duel', 'duelMapUID', '变异骷髅', 'quest_duel_beaten'},
}) do
    local f = fixture(case[1])
    f.state, f.monsterName = case[2], case[4]
    local callback = f.triggers.SYS_ON_KILL[1]
    f.playerMap = 42
    callback(1, 701)
    assert(f.state == case[2], case[1] .. ': kill accepted without saved map')
    f.runtime[case[3]] = 42
    f.playerMap = 43
    callback(1, 701)
    assert(f.state == case[2], case[1] .. ': kill accepted from another instance')
    f.playerMap = 42
    callback(1, 701)
    assert(f.state == case[5], case[1] .. ': in-scope kill rejected')
end

do
    local f = fixture('困魔咒任务')
    f.state = 'quest_in_rooms'
    for index = 1, 5 do f.runtime['roomMapUID' .. index] = 40 + index end
    f.monsterCounts[45] = 0
    local callback = f.triggers.SYS_ON_KILL[1]
    f.playerMap = 41
    callback(1, 801)
    assert(f.state == 'quest_in_rooms' and f.countQueries == 0 and #f.added == 0,
        '困魔咒: out-of-scope kill checked or completed the final room')
    f.playerMap, f.monsterCounts[45] = 45, 1
    callback(1, 801)
    assert(f.state == 'quest_in_rooms' and #f.added == 0 and f.gold == 0,
        '困魔咒: uncleared final room granted rewards')
    f.monsterCounts[45] = 0
    callback(1, 801)
    assert(f.state == 'SYS_DONE', '困魔咒: completion deferred to teacher')
    assert(#f.added == 2 and f.added[1][1] == '困魔咒（秘籍）' and f.added[1][2] == 1
        and f.added[2][1] == '黑除魔戒指' and f.added[2][2] == 1 and f.gold == 28000,
        '困魔咒: immediate rewards must be exactly one book, one ring, and 28000 gold')
    assert(#f.closed == 5 and f.states.quest_rooms_done == nil,
        '困魔咒: room cleanup missing or duplicate teacher-payout state remains')
    callback(1, 801)
    assert(#f.added == 2 and f.gold == 28000, '困魔咒: completed quest granted duplicate rewards')
    f.handlers['道馆_1/大悲善僧_1'].SYS_ENTER(1)
    assert(#f.added == 2 and f.gold == 28000, '困魔咒: teacher duplicated final-room rewards')
end

do
    local f = fixture('群体治愈术任务')
    f.state = 'quest_cave_done'
    f.states.quest_cave_done(1)
    local behavior = assert(f.behaviors['道馆_1/大悲善僧_1'])
    local args = table.pack(assert(load(behavior.args, 'behavior args', 't', f.env))())
    local handlers = assert(load(behavior.code, 'teacher behavior', 't', f.env))(
        table.unpack(args, 1, args.n))
    f.items['威魂深怨护身符'] = 1
    handlers.npc_return_charm(1)
    assert(f.items['威魂深怨护身符'] == 0 and #f.added == 1
        and f.added[1][1] == '神圣铂金戒指' and f.added[1][2] == 1 and f.gold == 43000,
        '群体治愈术: charm must be consumed for one ring and 43000 gold, not returned')
    local text = table.concat(f.posts[#f.posts].text)
    assert(text:find('重新送给你', 1, true), '群体治愈术: the legacy line was rewritten')
end

print('Taoist quest regression tests passed.')
