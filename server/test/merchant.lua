-- Run from the repository root: lua server/test/merchant.lua server/script/npc/*.lua
dofile('common/src/luamodule.lua')
package.path = 'server/script/?.lua;' .. package.path
package.preload['npc.include.merchant'] = function()
    error('typed merchants must not depend on a generic merchant builder')
end

SYS_ENTER = 'sys_enter'
SYS_EXIT = 'sys_exit'
SYS_DONE = 'sys_done'
SYS_ALLOWREDNAME = 'sys_allow_red_name'
SYS_LABEL = 'sys_label'
SYS_HIDE = 'sys_hide'
SYS_CHECKACTIVE = 'sys_check_active'
SYS_EPDEF = 'sys_epdef'
SYS_EPUID = 'sys_epuid'
SYS_EPQST = 'sys_epqst'
INVOP_TRADE = 1
INVOP_SECURE = 2
INVOP_REPAIR = 3
WLG_WEAPON = 1
LOGTYPE_WARNING = 1

local function read(path)
    local file = assert(io.open(path, 'r'))
    local text = file:read('a')
    file:close()
    return text
end

local items, itemIDs, itemTypes = {}, {}, {}
for name, kind in read('common/src/itemrecord.inc'):gmatch('%.name%s*=%s*u8"(.-)".-%.type%s*=%s*u8"(.-)"') do
    table.insert(items, {name = name, type = kind})
    itemIDs[name] = #items
    itemTypes[kind] = true
end
assert(#items > 0)

function assertType(value, ...)
    for _, expected in ipairs({...}) do
        if type(value) == expected or expected == 'integer' and math.type(value) == 'integer' then
            return
        end
    end
    error('unexpected type: ' .. type(value))
end
function fatalPrintf(fmt, ...) error(string.format(fmt, ...)) end
function getItemID(name) return itemIDs[name] or 0 end
function getItemName(id) return assert(items[id]).name end
function getNPCName() return 'test NPC' end
function getSubukGuildName() return 'test guild' end
function getTLSTable() return {} end
function getNanoTstamp() return 0 end
function isPlayer(uid) return uid == 1 end
function isQuest(uid) return uid == 2 end
function hasChar(text) return text:find('%S') ~= nil end
function tableEmpty(value) return value == nil or next(value) == nil end
function tableSize(value)
    local count = 0
    for _ in pairs(value) do count = count + 1 end
    return count
end
function splitString(text, delimiter)
    local parts = {}
    for part in text:gmatch('[^' .. delimiter .. ']+') do table.insert(parts, part) end
    return parts
end

-- Use the real NPC event dispatcher, mocking only its game-world dependencies.
dofile('server/src/npchar.lua')
local register = setEventHandler
local state, handler, captured
function addLog(level, fmt, ...)
    assert(level == LOGTYPE_WARNING)
    table.insert(state.warnings, string.format(fmt, ...))
end
function setEventHandler(value)
    assert(handler == nil, 'registered handlers twice')
    register(value)
    handler = value
end
function setNPCSell(goods)
    for _, name in ipairs(goods) do assert(getItemID(name) > 0, 'unknown stock: ' .. name) end
    state.goods = goods
end
function uidPostXML(uid, fmt, ...)
    state.xml = string.format(fmt, ...)
end
function uidQueryRedName(uid) return state.red end
function uidQueryName(uid) return 'test player' end
function uidQueryGold(uid) return state.gold end
function uidQueryItemDuration(uid, id, seq) return {5, 10} end
function uidRepairItem(uid, id, seq, special)
    state.repairs = state.repairs + 1
    state.special = special
    return true
end
function uidGrantGold(uid, gold) state.gold = state.gold + gold end
function uidRemove(uid, item)
    state.removed = state.removed + 1
    return true
end
function uidPostSell(uid) state.purchases = state.purchases + 1 end
function uidPostInvOpCost(uid, operation, id, seq, cost) state.cost = cost end
function uidPostStartInvOp(uid, operation, query, commit, types)
    assert(type(handler[query]) == 'function' and type(handler[commit]) == 'function')
    assert(#types > 0)
    for _, kind in ipairs(types) do assert(itemTypes[kind], 'unknown inventory type: ' .. kind) end
    state.operation = operation
    state.types = types
end
server = {player = {
    hasItem = function(uid, name, count)
        return (state.inventory[name] or 0) >= (count or 1)
    end,
    removeItem = function(uid, name, count)
        assert(getItemID(name) > 0)
        if (state.inventory[name] or 0) < count then return false end
        state.inventory[name] = state.inventory[name] - count
        return true
    end,
    addItem = function(uid, name, count)
        assert(getItemID(name) > 0)
        state.inventory[name] = (state.inventory[name] or 0) + count
    end,
    removeGold = function(uid, gold)
        assert(gold > 0)
        state.charges = state.charges + 1
        if state.failPayment or state.gold < gold then return false end
        state.gold = state.gold - gold
        return true
    end,
    getWLItem = function(uid, slot) return state.held end,
    removeWearItem = function(uid, slot)
        state.unbound = state.unbound + 1
        state.held = nil
        return true
    end,
}}

local playerAPI = require('api.player')
server.player.getLevel = playerAPI.getLevel
server.player.getJobList = playerAPI.getJobList
server.player.hasJob = playerAPI.hasJob
server.quest = require('api.quest')

-- Native runners can supply the actual Sol binding instead of the Lua fixture.
local nativeJobList = rawget(_G, 'MERCHANT_TEST_NATIVE_JOB_LIST')
local checkRPCValue = rawget(_G, 'MERCHANT_TEST_CHECK_RPC') or function(value)
    assert(canSerialize(value), 'RPC value is not serializable')
end

function uidRemoteCall(uid, ...)
    state.remoteCalls = state.remoteCalls + 1
    local args = table.pack(...)
    local code = args[args.n]
    for i = 1, args.n - 1 do checkRPCValue(args[i]) end
    local env = setmetatable({
        getGold = function() return state.gold end,
        getLevel = function() return state.level end,
        getJobList = function()
            local jobs = state.jobs or {state.job}
            if nativeJobList then
                return nativeJobList(jobs)
            end
            return jobs
        end,
        dbGetQuestState = function(uid, fsm) return state.questState end,
        removeGold = function(gold) return server.player.removeGold(uid, gold) end,
        hasItem = function(id, seq, count) return server.player.hasItem(uid, getItemName(id), count) end,
        removeItem = function(id, seq, count) return server.player.removeItem(uid, getItemName(id), count) end,
        addItem = function(id, count) return server.player.addItem(uid, getItemName(id), count) end,
    }, {__index = _G})
    local results = table.pack(assert(load(code, 'player remote call', 't', env))(table.unpack(args, 1, args.n - 1)))
    for i = 1, results.n do checkRPCValue(results[i]) end
    if state.deferRemoteReply then
        coroutine.yield('player remote response pending')
    end
    return table.unpack(results, 1, results.n)
end

local contracts =
{
    {name = 'smith',      setter = 'setSmith',      buy = true, sell = true, repair = true, special = true},
    {name = 'outfitter',  setter = 'setOutfitter',  buy = true, sell = true, repair = true},
    {name = 'jeweler',    setter = 'setJeweler',    buy = true, sell = true, repair = true},
    {name = 'apothecary', setter = 'setApothecary', buy = true, sell = true},
    {name = 'bookseller', setter = 'setBookseller', buy = true, sell = true},
    {name = 'grocer',     setter = 'setGrocer',     buy = true, sell = true},
    {name = 'butcher',    setter = 'setButcher',    buy = true, sell = true},
    {name = 'buyer',      setter = 'setBuyer',                 sell = true},
    {name = 'repairer',   setter = 'setRepairer',                            repair = true},
}
local modules = {}
for _, contract in ipairs(contracts) do
    local module = require('npc.include.merchant.' .. contract.name)
    local setter = module[contract.setter]
    module[contract.setter] = function(spec)
        captured = spec
        return setter(spec)
    end
    modules[contract.name] = {contract = contract, module = module}
end

local function reset()
    deleteEventHandler()
    handler, captured = nil, nil
    state = {gold = 10000, red = false, charges = 0, repairs = 0, purchases = 0, unbound = 0, removed = 0, inventory = {}, warnings = {}, remoteCalls = 0}
end
local function build(name, spec)
    reset()
    local entry = assert(modules[name])
    entry.module[entry.contract.setter](spec)
    assert(handler and handler[SYS_ALLOWREDNAME] == true)
end
local function call(tag, value, path)
    state.xml = nil
    if path == nil and tag ~= SYS_ENTER then path = SYS_EPDEF end
    _RSVD_NAME_npc_main(1, path, tag, value)
    assert(state.xml, 'event produced no dialogue: ' .. tag)
    for target in state.xml:gmatch('<event id="(.-)"') do
        assert(target == SYS_EXIT or type(handler[target]) == 'function', 'dangling event: ' .. target)
    end
end
local function contains(text)
    assert(state.xml:find(text, 1, true), 'missing dialogue: ' .. text)
end
local function fixture()
    return {
        greet = {'greeting 100%'},
        goods = {items[1].name},
        buyText = {'buy prompt'},
        sellText = {'sell prompt'},
        repairText = {'repair prompt'},
        repairDone = 'repair complete',
        specialText = {'special prompt'},
        redName = 'refusal',
        redNameExit = 'leave',
        today = 'daily fallback',
        todayExit = 'done',
        buyLabel = 'buy',
        buySuffix = ' buy suffix',
        sellLabel = 'sell',
        sellSuffix = ' sell suffix',
        repairLabel = 'repair',
        repairSuffix = ' repair suffix',
        specialLabel = 'special',
        specialSuffix = ' special suffix',
        exitLabel = 'exit',
        backLabel = 'return',
        topics = {{id = 'npc_topic', label = 'topic', text = {'topic 100%'}, back = 'topic return'}},
        extra = {[SYS_LABEL] = function(uid) return 'entry label' end},
    }
end

for _, contract in ipairs(contracts) do
    local spec = fixture()
    build(contract.name, spec)
    call(SYS_ENTER)
    contains('greeting 100%')
    assert((state.goods ~= nil) == (contract.buy or false))
    for operation, enabled in pairs({buy = contract.buy or false, sell = contract.sell or false,
        repair = contract.repair or false, special_repair = contract.special or false}) do
        assert((handler['npc_' .. operation] ~= nil) == enabled, contract.name .. ': ' .. operation)
        if enabled then
            local label = operation == 'special_repair' and 'special' or operation
            contains('id="npc_' .. operation .. '">' .. label .. '</event> ' .. label .. ' suffix')
        end
    end

    if contract.name == 'butcher' then
        assert(state.xml:find('id="npc_sell"', 1, true) < state.xml:find('id="npc_buy"', 1, true))
    end
    if contract.buy then
        call('npc_buy')
        contains('buy prompt')
        assert(state.purchases == 1)
    end
    if contract.sell then
        call('npc_sell')
        contains('sell prompt')
        assert(state.operation == INVOP_TRADE)
        local tradeItems = modules[contract.name].module.TRADE_ITEMS
        local value = tradeItems and getItemID(tradeItems[1]) .. ':1' or '1:1'
        call('npc_sell_query', value)
        call('npc_sell_commit', value)
        assert(state.removed == 1)
        if tradeItems then
            local gold = state.gold
            state.cost = nil
            call('npc_sell_query', '1:1')
            call('npc_sell_commit', '1:1')
            assert(state.cost == nil and state.removed == 1 and state.gold == gold)
        end
    end
    if contract.repair then
        call('npc_repair')
        contains('repair prompt')
        call('npc_repair_query', '1:1')
        call('npc_repair_commit', '1:1')
        contains('repair complete')
        assert(state.charges == 1 and state.repairs == 1 and state.special == false)
    end
    if contract.special then
        call('npc_special_repair')
        contains('special prompt')
        call('npc_special_query', '1:1')
        call('npc_special_commit', '1:1')
        contains('repair complete')
        assert(state.charges == 2 and state.repairs == 2 and state.special == true)
    end
    call('npc_topic')
    contains('topic 100%')
    contains('topic return')
    call('npc_today')
    contains('daily fallback')
    contains('id="' .. SYS_EXIT .. '" close="1">done')
    assert(not state.xml:find('id="' .. SYS_ENTER .. '"', 1, true))

    state.red = true
    assert(handler[SYS_LABEL](1) == 'entry label')
    local charges, repairs, purchases, removed = state.charges, state.repairs, state.purchases, state.removed
    for tag, callback in pairs(handler) do
        if type(callback) == 'function' and tag ~= SYS_LABEL then
            call(tag, '1:1')
            contains('refusal')
            contains('leave')
        end
    end
    assert(state.charges == charges and state.repairs == repairs and state.purchases == purchases and state.removed == removed)

    spec = fixture()
    spec.trade, spec.repair, spec.special, spec.today, spec.redName = false, false, false, nil, nil
    build(contract.name, spec)
    assert(handler.npc_sell == nil and handler.npc_repair == nil and handler.npc_special_repair == nil and handler.npc_today == nil)
    state.red = true
    call(SYS_ENTER)
    contains('greeting 100%')

    assert(not pcall(build, contract.name, {greet = {}}))
    local greetingCalls = 0
    build(contract.name, {greet = function(uid)
        greetingCalls = greetingCalls + 1
        return {'live greeting ' .. greetingCalls}
    end})
    call(SYS_ENTER)
    call(SYS_ENTER)
    contains('live greeting 2')
end

for name, modes in pairs({apothecary = {[0] = true, [3] = true}, buyer = {[52] = true}}) do
    local expected, actual = {}, {}
    for line in read('readme/sql2csv/King_StdItems.csv'):gmatch('[^\r\n]+') do
        local item, mode = line:match('^"%d+","(.-)","(%d+)"')
        if item and modes[tonumber(mode)] and getItemID(item) > 0 then expected[item] = true end
    end
    for _, item in ipairs(modules[name].module.TRADE_ITEMS) do actual[item] = true end
    for item in pairs(expected) do assert(actual[item], 'missing legacy trade item: ' .. item) end
    for item in pairs(actual) do assert(expected[item], 'unexpected legacy trade item: ' .. item) end
end

for _, potion in ipairs(modules.apothecary.module.SPECIAL_POTIONS) do
    local spec = fixture()
    spec[potion.flag] = true
    build('apothecary', spec)
    call(SYS_ENTER)
    contains(potion.item .. '<event id="' .. potion.tag .. '"')
    call(potion.tag)
    contains(potion.offer)
    state.gold = potion.price - 1
    call(potion.tag .. '_buy')
    assert(state.gold == potion.price - 1 and state.inventory[potion.item] == nil)
    state.gold = potion.price
    call(potion.tag .. '_buy')
    contains(potion.done)
    assert(state.gold == 0 and state.inventory[potion.item] == 1)
    call(potion.tag .. '_buy')
    assert(state.gold == 0 and state.inventory[potion.item] == 1)
    state.gold = potion.price
    call(potion.tag .. '_decline')
    contains(potion.decline)
    assert(state.gold == potion.price and state.inventory[potion.item] == 1)
end

local jeweler = modules.jeweler.module
local rustPrefix = '\u{751f}\u{9508}'
local function elementalItem(base, element)
    return base .. '\u{ff08}' .. element .. '\u{ff09}'
end
for _, element in ipairs(jeweler.RUST_ELEMENTS) do
    for index, base in ipairs(jeweler.RUST_ACCESSORIES) do
        local spec = fixture()
        spec.rustAccessory = true
        build('jeweler', spec)
        local rusty = rustPrefix .. base
        local reward = elementalItem(base, element.name)
        state.inventory = {[rusty] = 1}
        state.gold = jeweler.RUST_PRICE
        call('npc_rust_restore')
        contains('id="' .. element.tag .. '"')
        call(element.tag)
        assert(state.gold == 0 and state.inventory[rusty] == 0 and state.inventory[reward] == 1)
        if index == 9 and element.tag == 'npc_rust_dark' then
            contains(reward)
            assert(not state.xml:find('<event', 1, true))
        else
            contains(base .. '(' .. element.name .. ').')
            contains('id="' .. SYS_EXIT .. '"')
        end
        call(element.tag)
        assert(state.gold == 0 and state.inventory[reward] == 1)
    end
end

do
    local spec = fixture()
    spec.rustAccessory = true
    build('jeweler', spec)
    state.gold = 0
    call('npc_rust_restore')
    local noGold = state.xml
    state.gold = jeweler.RUST_PRICE
    call('npc_rust_restore')
    local noMaterial = state.xml
    assert(noGold ~= noMaterial)
    for _, base in ipairs(jeweler.RUST_ACCESSORIES) do state.inventory[rustPrefix .. base] = 1 end
    call('npc_rust_restore')
    local element = jeweler.RUST_ELEMENTS[1]
    state.gold = jeweler.RUST_PRICE - 1
    call(element.tag)
    assert(state.xml == noMaterial)
    for _, base in ipairs(jeweler.RUST_ACCESSORIES) do assert(state.inventory[rustPrefix .. base] == 1) end
    state.gold = jeweler.RUST_PRICE
    state.failPayment = true
    call(element.tag)
    assert(state.xml == noMaterial and state.gold == jeweler.RUST_PRICE)
    for _, base in ipairs(jeweler.RUST_ACCESSORIES) do assert(state.inventory[rustPrefix .. base] == 1) end
    state.failPayment = false
    call(element.tag)
    assert(state.inventory[rustPrefix .. jeweler.RUST_ACCESSORIES[1]] == 0)
    for index = 2, #jeweler.RUST_ACCESSORIES do
        assert(state.inventory[rustPrefix .. jeweler.RUST_ACCESSORIES[index]] == 1)
    end
    assert(state.gold == 0)
end

local repairType = items[getItemID(modules.smith.module.BOUND_SWORD[1])].type
for _, name in ipairs({'apothecary', 'bookseller', 'butcher', 'buyer'}) do
    local spec = fixture()
    spec.repair, spec.special = {repairType}, true
    build(name, spec)
    assert(handler.npc_repair == nil and handler.npc_special_repair == nil)
end

for _, name in ipairs({'grocer', 'repairer'}) do
    local spec = fixture()
    spec.repair = {repairType}
    build(name, spec)
    call('npc_repair_commit', '1:1')
    contains('repair complete')
    assert(state.repairs == 1)
end

for _, name in ipairs({'smith', 'outfitter'}) do
    local spec = fixture()
    spec.preRepairText = {'preliminary prompt'}
    build(name, spec)
    call(SYS_ENTER)
    contains('id="npc_pre_repair"')
    call('npc_pre_repair')
    contains('preliminary prompt')
    assert(state.operation == nil)
    call('npc_repair')
    contains('repair prompt')
    assert(state.operation == INVOP_REPAIR)
end

local spec = fixture()
spec.trade, spec.repair, spec.special = false, false, true
spec.exitLabel, spec.repairDoneBack = false, false
build('smith', spec)
assert(handler.npc_repair == nil and handler.npc_sell == nil)
call(SYS_ENTER)
assert(not state.xml:find('id="' .. SYS_EXIT .. '"', 1, true))
call('npc_special_commit', '1:1')
assert(state.special == true and state.charges == 1)
assert(not state.xml:find('<event', 1, true))

spec = fixture()
spec.repair, spec.special, spec.specialRepair = false, true, {repairType}
build('repairer', spec)
assert(handler.npc_repair == nil and handler.npc_buy == nil and handler.npc_sell == nil)
call('npc_special_commit', '1:1')
assert(state.special == true and state.charges == 1)

spec = fixture()
spec.qweapon, spec.removeSword = true, true
build('smith', spec)
local smith = modules.smith.module
call('npc_qweapon')
for _, line in ipairs(smith.QWEAPON) do contains(line) end
state.held = {itemID = getItemID(smith.BOUND_SWORD[1])}
call('npc_remove_sword')
assert(state.unbound == 1)
state.held = {itemID = 1}
call('npc_remove_sword')
assert(state.unbound == 1 and state.held.itemID == 1)

spec = fixture()
local book = modules.bookseller.module.BOOK_HELP[1]
spec.books = {book.name}
spec.booksText = {'first intro', 'second intro'}
spec.bookHelp = {[book.name] = {'original book prose'}}
build('bookseller', spec)
call('npc_book_explain')
contains('first intro')
contains('second intro')
call('npc_book_' .. book.tag:lower())
contains('original book prose')
contains('id="npc_book_explain"')

-- Load the real Wild Rush registrations, without starting its quest FSM or monster hooks.
local questName = '\u{91ce}\u{86ee}\u{51b2}\u{649e}\u{4efb}\u{52a1}'
local yimeiNPC = '\u{6bd4}\u{5947}\u{53bf}_0.\u{6021}\u{7f8e}_1'
local questRegistrations = {}
local questEnv = setmetatable({
    getUID = function() return 2 end,
    getQuestName = function() return questName end,
    getNPCharUID = function(map, npc) return map .. '.' .. npc end,
    setQuestFSMTable = function(fsm) assert(type(fsm) == 'table') end,
    uidRemoteCall = function(npc, ...)
        questRegistrations[npc] = table.pack(...)
    end,
    require = function(name)
        assert(name == 'quest.include.mondrop')
        return {setDropOnKill = function(spec) assert(type(spec) == 'table') end}
    end,
}, {__index = _G})
questEnv._G = questEnv
assert(loadfile('server/script/quest/' .. questName .. '.lua', 't', questEnv))()
assert(questRegistrations[yimeiNPC], 'missing Yimei quest registration')

local warrior = '\u{6218}\u{58eb}'
local wizard = '\u{6cd5}\u{5e08}'
local taoist = '\u{9053}\u{58eb}'
local questEntryCases =
{
    {jobs = {},                level = 27, visible = false},
    {jobs = {warrior},         level = 26, visible = false},
    {jobs = {warrior},         level = 27, visible = true},
    {jobs = {wizard},          level = 50, visible = false},
    {jobs = {taoist},          level = 50, visible = false},
    {jobs = {wizard, warrior}, level = 27, visible = true},
    {jobs = {warrior},         level = 27, visible = false, questState = SYS_DONE},
    {jobs = {warrior},         level = 27, visible = false, questState = SYS_ENTER},
}

local npcCount = 0
local questNPCCount = 0
for _, path in ipairs(arg) do
    if read(path):find("require%('npc%.include%.merchant%.%w+'%)") then
        reset()
        assert(loadfile(path))()
        assert(captured and handler, path)
        for tag, callback in pairs(handler) do
            if type(callback) == 'function' and tag ~= SYS_LABEL and tag ~= SYS_HIDE and tag ~= SYS_CHECKACTIVE then
                call(tag, '1:1')
            end
        end
        if captured.repairDone and handler.npc_repair_commit then
            call('npc_repair_commit', '1:1')
            contains(captured.repairDone)
        end
        if handler.npc_hntl_receive then
            -- Pan Ye spirit exchange: professions, replay and cancellation at an RPC boundary.
            local spirit = '\u{6f58}\u{591c}\u{5929}\u{7075}'
            local missingSpiritRecord = getItemID(spirit) == 0
            if missingSpiritRecord then
                assert(#state.warnings > 0)
                call('npc_hntl_receive')
                assert(next(state.inventory) == nil)
                -- The source archive has no item record. Exercise its retained branch with
                -- a test-only record instead of assigning invented game data to the token.
                table.insert(items, {name = spirit, type = items[1].type})
                itemIDs[spirit] = #items
                reset()
                assert(loadfile(path))()
            end
            for _, case in ipairs({
                {'\u{6218}\u{58eb}', '\u{6f58}\u{591c}\u{70bc}\u{72f1}'},
                {'\u{6cd5}\u{5e08}', '\u{6f58}\u{591c}\u{9b54}\u{6756}'},
                {'\u{9053}\u{58eb}', '\u{6f58}\u{591c}\u{94f6}\u{86c7}'},
            }) do
                state.inventory = {[spirit] = 1}
                state.job = case[1]
                call(SYS_ENTER)
                contains('id="npc_hntl_01"')
                assert(not state.xml:find('id="npc_buy"', 1, true))
                local remoteCalls = state.remoteCalls
                call('npc_hntl_receive')
                assert(state.remoteCalls == remoteCalls + 1)
                assert(state.inventory[spirit] == 0 and state.inventory[case[2]] == 1)
                call('npc_hntl_receive')
                assert(state.inventory[case[2]] == 1)

                state.inventory = {[spirit] = 1}
                state.deferRemoteReply = true
                local pending = coroutine.create(function() call('npc_hntl_receive') end)
                local resumed, reason = coroutine.resume(pending)
                assert(resumed, reason)
                assert(reason == 'player remote response pending')
                assert(coroutine.status(pending) == 'suspended')
                assert(coroutine.close(pending))
                state.deferRemoteReply = false
                assert(state.inventory[spirit] == 0 and state.inventory[case[2]] == 1)
                call('npc_hntl_receive')
                assert(state.inventory[case[2]] == 1)
            end
            state.inventory = {[spirit] = 1}
            state.jobs = {'\u{6cd5}\u{5e08}', '\u{6218}\u{58eb}'}
            state.red = true
            local remoteCalls = state.remoteCalls
            call('npc_hntl_receive')
            contains(captured.redName)
            assert(state.inventory[spirit] == 1 and state.remoteCalls == remoteCalls)
            state.red = false
            call('npc_hntl_receive')
            assert(state.inventory['\u{6f58}\u{591c}\u{70bc}\u{72f1}'] == 1)
            state.jobs = nil
            state.inventory = {}
            if missingSpiritRecord then
                itemIDs[spirit] = nil
                table.remove(items)
            end
        end
        for tag, field in pairs({
            [SYS_ENTER] = 'greet', npc_buy = 'buyText', npc_sell = 'sellText',
            npc_pre_repair = 'preRepairText', npc_repair = 'repairText', npc_today = 'today',
        }) do
            local text = captured[field]
            if handler[tag] and text then
                call(tag)
                if type(text) == 'function' then text = text(1) end
                for _, line in ipairs(type(text) == 'table' and text or {text}) do contains(line) end
            end
        end
        local registration = questRegistrations[path:match('([^/]+)%.GLOC_')]
        if registration then
            assert(load(registration[registration.n], 'NPC quest registration', 't', _G))(
                table.unpack(registration, 1, registration.n - 1))
            for _, case in ipairs(questEntryCases) do
                state.jobs, state.level, state.questState = case.jobs, case.level, case.questState
                call(SYS_ENTER)
                local questPath = 'path="' .. SYS_EPQST .. '/' .. questName .. '"'
                assert((state.xml:find(questPath, 1, true) ~= nil) == case.visible, path)
                local jobs = server.player.getJobList(1)
                assert(type(jobs) == 'table' and getmetatable(jobs) == nil and #jobs == #case.jobs)
                for i, job in ipairs(case.jobs) do assert(jobs[i] == job) end
                if case.visible then call(SYS_ENTER, nil, SYS_EPDEF) end
                local greeting = captured.greet
                if type(greeting) == 'function' then greeting = greeting(1) end
                for _, line in ipairs(greeting) do contains(line) end
            end
            deleteQuestHandler(questName)
            questNPCCount = questNPCCount + 1
        end
        npcCount = npcCount + 1
    end
end
print(string.format('Passed %d merchant contracts and %d NPC scripts', #contracts, npcCount))
print(string.format('Passed %d quest-aware merchant entry paths', questNPCCount))

return
{
    loadNPC = function(path)
        reset()
        assert(loadfile(path))()
        return captured
    end,
    post = function(tag, value)
        call(tag, value)
        return state.xml
    end,
}
