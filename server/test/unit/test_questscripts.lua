local root = assert(arg[1], 'Repository root is required')
local STOP = {}

local function fixture(file)
    local f = {items = {}, quality = {}, flags = {}, handlers = {}, registrations = {}, state = nil, level = 1, job = '道士', descriptions = {}, vars = {}, triggers = {}}
    local env = setmetatable({}, {__index = _G})
    env._G = env
    for _, key in ipairs({'SYS_ENTER', 'SYS_EXIT', 'SYS_DONE', 'SYS_LUANIL', 'SYS_EPUID', 'SYS_EPQST', 'SYS_QSTFSM', 'SYS_CHECKACTIVE', 'SYS_LABEL', 'SYS_HIDE', 'SYS_ON_GAINITEM', 'SYS_ON_KILL', 'SYS_ON_DIE', 'SYS_ON_OFFLINE'}) do
        env[key] = key
    end
    env.SYS_GOLDNAME = '金币'
    env.SYS_QUESTFIELD = {TEAM = {ROLELIST = 'roles'}}
    env.WLG_DRESS = 1
    env.getUID = function() return 91 end
    env.getQuestName = function() return file end
    env.getItemID = function(name) return name end
    env.getItemName = function(name) return name end
    env.getNPCharUID = function(map, npc) return map .. '/' .. npc end
    env.asInitString = function(v)
        local function encode(x)
            if type(x) == 'table' then
                local parts = {}
                for k, value in pairs(x) do
                    parts[#parts + 1] = '[' .. encode(k) .. ']=' .. encode(value)
                end
                return '{' .. table.concat(parts, ',') .. '}'
            end
            return type(x) == 'string' and string.format('%q', x) or tostring(x)
        end
        return encode(v)
    end
    env.setQuestFSMTable = function(states) f.states = states end
    env.dbGetQuestVar = function(uid, key) return f.vars[key] end
    env.dbSetQuestVar = function(uid, key, value) f.vars[key] = value end
    env.setQuestState = function(args)
        if args.from then
            local matches = args.from == f.state or (args.from == env.SYS_LUANIL and f.state == nil)
            if type(args.from) == 'table' then
                for _, state in ipairs(args.from) do
                    matches = matches or state == f.state or (state == env.SYS_LUANIL and f.state == nil)
                end
            end
            if not matches then return false end
        end
        f.state = args.state
        if f.inState then error(STOP) end
        return true
    end
    env.setQuestDesp = function(args)
        f.descriptions[#f.descriptions + 1] = string.format(table.unpack(args))
    end
    local player = {}
    player.hasJob = function(uid, job) return f.job == job end
    player.getLevel = function() return f.level end
    player.getName = function() return '少侠' end
    player.getGender = function() return true end
    player.dbHasFlag = function(uid, key) return f.flags[key] == true end
    player.dbAddFlag = function(uid, key) f.flags[key] = true end
    player.hasItem = function(uid, name, count, count2) return (f.items[name] or 0) >= (count2 or count) end
    player.removeItem = function(uid, name, count, count2)
        count = count2 or count
        if not player.hasItem(uid, name, count) then return false end
        f.items[name] = f.items[name] - count
        return true
    end
    player.addItem = function(uid, name, count) f.items[name] = (f.items[name] or 0) + (count or 1) end
    player.hasItemQuality = function(uid, name, quality, count)
        return player.hasItem(uid, name, count) and (f.quality[name] or 0) >= quality
    end
    player.removeItemQuality = function(uid, name, quality, count)
        return player.hasItemQuality(uid, name, quality, count) and player.removeItem(uid, name, count)
    end
    player.getWLItem = function() return f.dress and {itemID = f.dress} end
    env.server = {player = player, quest = {
        setState = function(uid, args) return env.setQuestState(args) end,
        getState = function() return f.state end,
        setDesp = function(uid, args) env.setQuestDesp(args) end,
    }}
    env.getLevel = player.getLevel
    env.hasItem = function(...) return player.hasItem(11, ...) end
    env.postString = function() end
    env.addTrigger = function(kind, callback) f.triggers[#f.triggers + 1] = callback; return #f.triggers end
    env.deleteTrigger = function() end
    env.getThreadAddress = function() return 123 end
    env.waitNotify = function() return false end
    env.pause = function() end
    env.setQuestHandler = function(name, handler)
        f.registration = handler
        f.registrations[f.remoteUID] = handler
    end
    local dialog = {
        post = function(uid, ...) f.dialogue = {...} end,
        link = function(id, label) return {id = id, label = label} end,
    }
    env.require = function(name)
        if name == 'include.dialog' then return dialog end
        if name == 'quest.include.mondrop' then return {addDropTrigger = function() end} end
        error('Unexpected module: ' .. name)
    end
    env.uidRemoteCall = function(uid, ...)
        local args = table.pack(...)
        f.remoteUID = uid
        return assert(load(args[args.n], file, 't', env))(table.unpack(args, 1, args.n - 1))
    end
    env.setupNPCQuestBehavior = function(map, npc, uid, argstr, code)
        local args = argstr and table.pack(assert(load(argstr, file, 't', env))()) or table.pack()
        f.handlers[npc] = assert(load(code, file, 't', env))(table.unpack(args, 1, args.n))
    end
    function f:enter(state)
        self.state = state
        self.inState = true
        local ok, err = pcall(assert(self.states[state]), 11)
        self.inState = false
        assert(ok or err == STOP, err)
    end
    assert(loadfile(root .. '/server/script/quest/' .. file .. '.lua', 't', env))()
    f.env = env
    return f
end

-- 道士/战士/法师初出江湖 post their dialogs through the real include/dialog.lua, each one is checked as the NPC's XML parser would see it
local function checkXML(xml)
    for tag in xml:gmatch('<([^>]*)>') do
        local rest = tag:gsub('^/?[%w_]+', '', 1):gsub('%s+[%w_]+="[^"]*"', '')
        assert(rest:match('^%s*/?$'), 'malformed tag: <' .. tag .. '>')
    end
end

local function introFixture(t, level)
    local f = fixture(t.quest)
    f.job, f.level = t.job, level or 1

    local denv = setmetatable({
        SYS_EXIT = 'SYS_EXIT', SYS_EPUID = 'SYS_EPUID', SYS_EPQST = 'SYS_EPQST', SYS_EPDEF = 'SYS_EPDEF',
        assertType = function(v, ...)
            for _, t in ipairs({...}) do
                if type(v) == t or (t == 'integer' and math.type(v) == 'integer') then return v end
            end
            error('unexpected type: ' .. type(v), 2)
        end,
        isArray = function(t)
            if type(t) ~= 'table' then return false end
            local n = 0
            for _ in pairs(t) do n = n + 1 end
            return n == #t
        end,
        uidPostXML = function(uid, path, fmt, xml)
            checkXML(xml)
            f.xml = xml
        end,
    }, {__index = _G})
    local dialog = assert(loadfile(root .. '/server/script/include/dialog.lua', 't', denv))()

    f.env.require = function(name)
        assert(name == 'include.dialog', 'Unexpected module: ' .. name)
        return dialog
    end
    f.env.clearNPCQuestBehavior = function(map, npc, uid) f.handlers[npc] = nil end
    f.env.spaceMove = function(target) f.moved = target end

    -- again, the guide registers with the real dialog this time
    assert(loadfile(root .. '/server/script/quest/' .. t.quest .. '.lua', 't', f.env))()
    f.guide = f.registrations[t.guide]
    return f
end

local function posted(f, text)
    return f.xml ~= nil and f.xml:find(text, 1, true) ~= nil
end

local intros =
{
    {
        quest = '道士初出江湖', job = '道士', guide = '道馆_1/士官_1', guideNPC = '士官_1', shop = '大老板_1', recipient = '阿潘_1',
        shopTarget = "{'道馆_1',394,169}", recipientTarget = "{'道馆_1',429,120}",
        parcel = '道力护身符', material = '鸡血', count = 2, book = '治愈术',
        wait = '请级别高一点，修练到6级以上再来吧！',
        menuLow = '好的。首先贫道会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下道馆内的事情！',
        menuHigh = '好的。首先贫道会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下道馆内的事情！',
        jobText = '主动直接与敌人交手违背了我们上仙药手的教诲。因此在战斗中我们主要采取防御保护的方式。',
        noParcel = '道力护身符在哪啊？',
        askHowToCollect = false, collected = '嗯，只要去猎到鸡自然就会有鸡血了，所以不用特别担心！',
        handInFail = '去猎几只鸡，鸡血自然就会有了！',
        askedGuide = '你找过<t color="red">士官</t>了吗？',
        notYet6 = '下次交给你做的事可能有点难，等你修炼一段时间后再来吧！',
        goWang = '去见完比奇省的王大人，还要请您去拜访本馆的清明子！',
        wangGreet = '来此有何贵干啊？', wangAsk = '难道你是受道馆的士官之托而来的人？',
        wangNoBook = '可是道馆的士官让你转交给我的书呢？', wangDeny = '不知是不是在哪儿遇到了什么麻烦<t wrap="0">···</t>',
        offerDecline = '请让我考虑一下。',
    },

    {
        quest = '战士初出江湖', job = '战士', guide = '边境城市_01/上官小姐_1', guideNPC = '上官小姐_1', shop = '肉店金老板_1', recipient = '德秀_1',
        shopTarget = "{'边境城市_01',425,274}", recipientTarget = "{'边境城市_01',459,279}",
        parcel = '肉汤', material = '牛肉', count = 1, quality = 10, book = '基本剑术',
        wait = '等级别再高一点，达到了6级再来吧。',
        menuLow = '那么，首先我会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下这个边境城市。',
        menuHigh = '那么首先呢，我会教你做一些简单容易做的事情，想要试试吗？',
        jobText = '（不过作为男人还是要有力气才行！不是吗？呵呵呵<t wrap="0">···</t>）',
        jobFemale = '（也就是说，如果是女子的话也不是不行的。）',
        noParcel = '肉汤在哪？',
        askHowToCollect = true, collected = '好吧，现在就出去寻找牛，然后把<t color="red">质量在10以上的牛肉</t>拿来！',
        handInFail = '嗯？看来这不是<t color="red">质量在10以上的牛肉</t>啊？',
        askedGuide = '你找过<t color="red">上官小姐</t>了？有什么事？',
        notYet6 = '下次交给你做的事可能有点难，等你修炼一段时间后再来吧！',
        goWang = '去拜访比奇省的王大人，在去之前请先去拜访一下龙血先生。',
        wangGreet = '找我有什么事吗？', wangAsk = '难道你是受边境城市上官小姐之托而来的人？',
        wangNoBook = '可是上官小姐让你转交给我的书呢？', wangDeny = '担心会不会出什么事儿啊！',
        offerDecline = '给我点时间考虑一下吧！',
    },

    {
        quest = '法师初出江湖', job = '法师', guide = '银杏山谷_02/南宫小姐_1', guideNPC = '南宫小姐_1', shop = '许氏_1', recipient = '铁匠师傅_1',
        shopTarget = "{'银杏山谷_02',228,194}", recipientTarget = "{'银杏山谷_02',284,197}",
        parcel = '肉汤', material = '鸡肉', count = 1, quality = 4, book = '火球术',
        wait = '等级别高一点，修炼到6级再来找我吧！',
        menuLow = '那么，首先我会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下银杏山谷吧。',
        menuHigh = '嗯，<t color="red">少侠</t>。',
        jobText = '仅靠修炼是不能成为法神的，作为法神一定要拥有法神的资质，主要是从血统上来继承这种资质的。',
        noParcel = '肉汤在哪？',
        askHowToCollect = true, collected = '好了，现在就出去找<t color="red">质量4以上的鸡肉</t>吧！',
        handInFail = '看起来不是高质量的鸡肉啊！',
        askedGuide = '找过<t color="red">南宫小姐</t>了吗？有什么事吗？',
        notYet6 = '下次交给你的任务可能会有点难，再修炼一段时间再来吧。',
        goWang = '拜访住在比奇省的王大人后，另外还要找一下霹雳尊者。',
        wangGreet = '找我有什么事吗？', wangAsk = '难道你是受银杏山谷南宫小姐之托而来的人？',
        wangNoBook = '那么南宫小姐叫你转交给我的书呢？', wangDeny = '可能是在哪儿遇到了什么麻烦<t wrap="0">···</t>',
        offerDecline = '请给我点时间考虑一下。',
    },
}

for _, t in ipairs(intros) do
    local f = introFixture(t)

    -- one menu entry, 初出江湖, for its own profession and only before the quest starts
    assert(f.guide.SYS_LABEL == '初出江湖')
    assert(f.guide.SYS_CHECKACTIVE(11))
    f.job = '其他职业'
    assert(not f.guide.SYS_CHECKACTIVE(11))
    f.job = t.job
    f.state = 'SYS_DONE'
    assert(not f.guide.SYS_CHECKACTIVE(11))
    f.state = nil

    for _, level in ipairs({1, 3, 4, 5, 6}) do
        f.level = level
        f.guide.SYS_ENTER(11)
        assert(posted(f, 'id="npc_quest"') == (level < 4 or level >= 6))
        assert(posted(f, (level >= 4 and level < 6) and t.wait or (level >= 6 and t.menuHigh or t.menuLow)))
    end

    f.guide.npc_job(11)
    assert(posted(f, t.jobText))
    if t.jobFemale then
        f.env.server.player.getGender = function() return false end
        f.guide.npc_job(11)
        assert(posted(f, t.jobFemale))
    end

    -- below level 4 the errands, accepted once
    f.level = 1
    f.guide.npc_quest(11)
    assert(posted(f, 'id="npc_accept_errand"') and posted(f, 'args="' .. t.shopTarget .. '"'))
    f.guide.npc_fly_to_loc(11, f.xml:match('args="([^"]*)"'))
    local target = load('return ' .. t.shopTarget)()
    assert(f.moved[1] == target[1] and f.moved[2] == target[2] and f.moved[3] == target[3])
    f.guide.npc_accept_errand(11)
    assert(f.state == 'SYS_ENTER')
    f.state = 'quest_deliver_parcel'
    f.guide.npc_accept_errand(11)
    assert(f.state == 'quest_deliver_parcel')

    -- [102]: the guide asks after the shop, the shop hands out the parcel after its switch
    f:enter('SYS_ENTER')
    f.handlers[t.guideNPC].SYS_ENTER(11)
    assert(posted(f, 'args="' .. t.shopTarget .. '"'))
    local shop = f.handlers[t.shop]
    shop.SYS_ENTER(11)
    assert(posted(f, 'id="npc_accept"'))
    f.state = 'quest_deliver_parcel'
    shop.npc_accept(11)
    assert(f.items[t.parcel] == nil)
    f.state = 'SYS_ENTER'
    shop.npc_accept(11)
    assert(f.state == 'quest_deliver_parcel' and f.items[t.parcel] == 1 and posted(f, 'args="' .. t.recipientTarget .. '"'))

    -- [103]: check, switch, take
    f:enter('quest_deliver_parcel')
    f.handlers[t.shop].SYS_ENTER(11)
    assert(posted(f, 'args="' .. t.recipientTarget .. '"'))
    local recipient = f.handlers[t.recipient]
    f.items[t.parcel] = 0
    recipient.SYS_ENTER(11)
    assert(posted(f, t.noParcel) and f.items['匕首'] == nil)
    f.items[t.parcel] = 1
    f.state = 'quest_request_material'
    recipient.SYS_ENTER(11)
    assert(f.items[t.parcel] == 1 and f.items['匕首'] == nil)
    f.state = 'quest_deliver_parcel'
    recipient.SYS_ENTER(11)
    assert(f.state == 'quest_request_material' and f.items[t.parcel] == 0 and f.items['匕首'] == 1)

    -- [163]: the recipient sends the player back, the shop asks for the material
    f:enter('quest_request_material')
    f.handlers[t.recipient].SYS_ENTER(11)
    assert(posted(f, 'args="' .. t.shopTarget .. '"'))
    shop = f.handlers[t.shop]
    if t.askHowToCollect then
        shop.SYS_ENTER(11)
        assert(f.state == 'quest_request_material' and posted(f, 'id="npc_tell_how"') and posted(f, 'id="npc_known"'))
        shop.npc_tell_how(11)
        assert(f.state == 'quest_collect_material' and posted(f, t.collected))
        f.xml = nil
        shop.npc_known(11)
        assert(f.xml == nil)
    else
        shop.SYS_ENTER(11)
        assert(f.state == 'quest_collect_material' and posted(f, t.collected))
        f.xml = nil
        shop.SYS_ENTER(11)
        assert(f.xml == nil)
    end

    -- [104]: the recipient goes quiet, the hand-in checks, switches, takes and pays
    f:enter('quest_collect_material')
    assert(f.handlers[t.recipient] == nil)
    shop = f.handlers[t.shop]
    shop.SYS_ENTER(11)
    assert(posted(f, 'id="npc_hand_in"'))
    f.items[t.material] = t.count - 1
    f.quality[t.material] = t.quality
    shop.npc_hand_in(11)
    assert(posted(f, t.handInFail) and f.items['金币'] == nil)
    f.items[t.material] = t.count
    if t.quality then
        f.quality[t.material] = t.quality - 1
        shop.npc_hand_in(11)
        assert(posted(f, t.handInFail) and f.items['金币'] == nil)
        f.quality[t.material] = t.quality
    end
    f.state = 'quest_prepare_to_wang'
    shop.npc_hand_in(11)
    assert(f.items[t.material] == t.count and f.items['金币'] == nil)
    f.state = 'quest_collect_material'
    shop.npc_hand_in(11)
    assert(f.state == 'quest_prepare_to_wang' and f.items[t.material] == 0 and f.items['金币'] == 1000)

    -- [105]: the guide gives the book at level 6, after its switch
    f:enter('quest_prepare_to_wang')
    f.handlers[t.shop].SYS_ENTER(11)
    assert(posted(f, t.askedGuide))
    local guide = f.handlers[t.guideNPC]
    guide.SYS_ENTER(11)
    assert(posted(f, 'id="npc_ask_more"'))
    f.level = 5
    guide.npc_ask_more(11)
    assert(posted(f, t.notYet6))
    f.level = 6
    guide.npc_ask_more(11)
    assert(posted(f, 'id="npc_accept_book"'))
    f.state = 'quest_deliver_book'
    guide.npc_accept_book(11)
    assert(f.items['古籍'] == nil and f.items[t.book] == nil)
    f.state = 'quest_prepare_to_wang'
    guide.npc_accept_book(11)
    assert(f.state == 'quest_deliver_book' and f.items['古籍'] == 1 and f.items[t.book] == 1)

    -- [131]: the shop goes quiet, 王大人 asks without the book, takes it after the switch
    f:enter('quest_deliver_book')
    assert(f.handlers[t.shop] == nil)
    f.handlers[t.guideNPC].SYS_ENTER(11)
    assert(posted(f, t.goWang))
    local wang = f.handlers['王大人_1']
    wang.SYS_ENTER(11)
    assert(posted(f, t.wangGreet))
    f.items['古籍'] = 0
    wang.npc_hand_in(11)
    assert(posted(f, t.wangAsk) and posted(f, 'id="npc_no_book"') and posted(f, 'id="npc_deny"') and f.items['青铜头盔'] == nil)
    wang.npc_no_book(11)
    assert(posted(f, t.wangNoBook))
    wang.npc_deny(11)
    assert(posted(f, t.wangDeny))
    f.items['古籍'] = 1
    f.state = 'SYS_DONE'
    wang.npc_hand_in(11)
    assert(f.items['古籍'] == 1 and f.items['青铜头盔'] == nil and not f.flags.done_wang_book)
    f.state = 'quest_deliver_book'
    wang.npc_hand_in(11)
    assert(f.state == 'SYS_DONE' and f.items['古籍'] == 0 and f.items['金币'] == 2000 and f.items['青铜头盔'] == 1)
    assert(f.flags.done_wang_book)
    f.states.SYS_DONE(11)
    assert(f.descriptions[#f.descriptions]:find('青铜头盔', 1, true))

    -- level 6: the offer, the explanation, then the book, given once
    f = introFixture(t, 6)
    f.guide.npc_quest(11)
    assert(posted(f, 'id="npc_explain"') and posted(f, t.offerDecline) and posted(f, '<t color="red">少侠</t>'))
    f.guide.npc_explain(11)
    assert(posted(f, 'id="npc_accept_book"'))
    f.state = 'SYS_ENTER'
    f.guide.npc_accept_book(11)
    assert(f.items['古籍'] == nil)
    f.state = nil
    f.guide.npc_accept_book(11)
    assert(f.state == 'quest_deliver_book' and f.items['古籍'] == 1 and f.items[t.book] == 1)
end

for _, event in ipairs({'npc_accept_ring', 'npc_ask_why_not_go_directly'}) do
    local f = fixture('气霖证书')
    f:enter('quest_ask_wife')
    local h = f.handlers['苏百花_1']
    h[event](11)
    assert(f.items['玉指环'] == nil)
    f.items['气霖证书'] = 1
    h[event](11)
    assert(f.items['气霖证书'] == 0 and f.items['玉指环'] == 1)
    f:enter('quest_ask_husband')
    assert(f.descriptions[#f.descriptions]:find('洪气霖', 1, true))
    f.handlers['洪气霖_1'].npc_give_ring(11)
    assert(f.items['玉指环'] == 0 and f.items['制魔宝玉'] == 1 and f.state == 'SYS_DONE')
end

do
    local f = fixture('轻型盔甲任务')
    assert(not f.registration.SYS_CHECKACTIVE(11))
    f.flags.done_wang_coc = true
    assert(f.registration.SYS_CHECKACTIVE(11))
    f.level = 11
    for dress, expected in pairs({
        ['布衣（男）'] = '穿着很是稀松平常',
        ['布衣（女）'] = '穿着很是稀松平常',
        ['轻型盔甲（男）'] = '已经穿上轻型盔甲',
        ['轻型盔甲（女）'] = '已经穿上轻型盔甲',
    }) do
        f.dress = dress
        f.registration.SYS_ENTER(11)
        assert(table.concat(f.dialogue[2]):find(expected, 1, true))
    end
    f.items['铁矿'] = 5
    f.quality['铁矿'] = 12
    f:enter('SYS_ENTER')
    assert(f.state == 'SYS_ENTER')
    f.quality['铁矿'] = 13
    f:enter('SYS_ENTER')
    assert(f.state == 'quest_got_iron')
    f:enter('quest_got_iron')
    f.handlers['怡美_1'].SYS_ENTER(11)
    assert(f.items['铁矿'] == 0 and f.items['耐久轻型盔甲（男）'] == 1)
end

do
    -- the ore trigger runs in the player actor: a server.player call there is a remote call to itself, which raises
    local f = fixture('轻型盔甲任务')
    f:enter('SYS_ENTER')
    assert(f.state == 'SYS_ENTER' and #f.triggers == 1)

    local player = f.env.server.player
    local hasItemQuality = player.hasItemQuality
    f.env.hasItemQuality = function(item, quality, count) return hasItemQuality(11, item, quality, count) end
    player.hasItemQuality = function() error('Sending remote call to self is not allowed') end

    f.items['铁矿'], f.quality['铁矿'] = 5, 12
    assert(not f.triggers[1]('铁矿') and f.state == 'SYS_ENTER', 'ore below purity 13 counted')
    f.quality['铁矿'] = 13
    assert(f.triggers[1]('铁矿') == true and f.state == 'quest_got_iron', 'five pure ores gained do not move the quest')
end

do
    local f = fixture('苍蝇拍任务')
    f.items['牛毛'] = 1
    f.items['竹棍'] = 1
    f:enter('quest_start_collection')
    assert(f.state == 'quest_complete_collection')
    f:enter('quest_complete_collection')
    f.items['竹棍'] = 0
    f.handlers['杂货商_1'].SYS_ENTER(11)
    assert(f.items['苍蝇拍'] == nil)
    -- @PARICHE_JABSANG_3: pariche_12 with the 牛毛 only, pariche_11 without it
    assert(f.dialogue[2]:find('好像已经找到了', 1, true), '苍蝇拍: missing 竹棍 answered without pariche_12')
    f:enter('quest_complete_collection')
    f.items['牛毛'] = 0
    f.handlers['杂货商_1'].SYS_ENTER(11)
    assert(f.dialogue[2]:find('年轻人真是磨蹭啊', 1, true), '苍蝇拍: missing 牛毛 answered without pariche_11')
    f.items['牛毛'] = 1
    f.items['竹棍'] = 1
    f:enter('quest_complete_collection')
    f.handlers['杂货商_1'].SYS_ENTER(11)
    assert(f.items['苍蝇拍'] == 1 and f.items['牛毛'] == 0 and f.items['竹棍'] == 0)
end

do
    local f = fixture('队友合作')
    f:enter('quest_setup_kill_trigger')
    assert(f.state == 'quest_failed')
    f:enter('quest_failed')
    assert(f.descriptions[#f.descriptions]:find('重新挑战', 1, true))
end

do
    local env = setmetatable({
        SYS_ON_BEGIN = 0, SYS_ON_END = 0, SYS_DONE = 'done', SYS_QSTFSM = 'main',
        getUID = function() return 11 end,
        assertType = function() end,
        _RSVD_NAME_trigger = function() end,
        _RSVD_NAME_callFuncCoop = function() return {91} end,
    }, {__index = _G})
    local description = {main = '完成了原来的委托。'}
    local reported
    env.uidRemoteCall = function(uid, playerUID, code)
        if code:find('_RSVD_NAME_loadQuestContext', 1, true) then return end
        return 'completed', env.SYS_DONE, description
    end
    env._RSVD_NAME_reportQuestDespList = function(list) reported = list end
    assert(loadfile(root .. '/server/src/player.lua', 't', env))()
    env._RSVD_NAME_setupQuests()
    assert(reported.completed.main == description.main)
    description = nil
    env._RSVD_NAME_setupQuests()
    assert(reported.completed.main == '任务已完成')
end

print('Quest scripts passed: introductory routes, hand-ins, certificate, qualified ore, material readiness, challenge timeout, and completed descriptions.')
