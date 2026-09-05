local dialogue = require('npc.include.dialog')
local invop = require('npc.include.invop')

-- 杂货店, legacy Market_Def/07Grocery_*.txt; chestnut barter uses its own NPC scripts.
--
-- the general store: candles, torches, scrolls, repair oil and charms. buys and sells, and one
-- of the 16 also repairs, so repair is off by default and that one turns it on
--
--     local grocer = require('npc.include.merchant.grocer')
--     grocer.setGrocer
--     {
--         greet = {'你要买什么？'},
--         goods = {'蜡烛', '火把', '地牢逃脱卷'},
--         repair = {'衣服', '武器'},         -- only 07Grocery_Bichon-0 does this
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local grocer = {}

function grocer.setGrocer(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty grocer greeting')

    local label = spec.label or '物品'
    local trade = optList(spec.trade, {'火把', '传送卷轴', '功能药水', '护身符', '药粉', '道具'})
    local price = spec.price or 50
    local repairDoneBack = spec.repairDoneBack
    if repairDoneBack == nil then repairDoneBack = spec.backLabel end
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '购买', {suffix = spec.buySuffix or label}))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'你要买什么？'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', {suffix = spec.sellSuffix or label}))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'请把不用的东西卖给我'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
    end

    -- Ordinary mending is an explicit service at the Bichon general store, not a grocer default.
    if spec.repair then
        table.insert(menu, dialogue.link('npc_repair', spec.repairLabel or '修理', {suffix = spec.repairSuffix or label}))
        handler.npc_repair = function(uid, value)
            dialogue.post(uid, spec.repairText or {'这里可以修理衣服和武器之类的东西。'}, back)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', spec.repair)
        end
        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', spec.repair)
        end
        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', spec.repair, spec.repairDone, repairDoneBack)
        end
    end

    for _, topic in ipairs(spec.topics or {}) do
        table.insert(menu, dialogue.link(topic.id, topic.label, {prefix = topic.prefix, suffix = topic.suffix}))
        handler[topic.id] = topic.handler or function(uid, value)
            dialogue.post(uid, topic.text, {dialogue.link(SYS_ENTER, topic.back or spec.backLabel or '前一步')})
        end
    end

    if spec.today then
        table.insert(menu, dialogue.link('npc_today', '对今日的任务进行了解'))
        handler.npc_today = function(uid, value)
            dialogue.post(uid, {spec.today}, {dialogue.link(SYS_EXIT, spec.todayExit or '结束')})
        end
    end

    table.insert(menu, dialogue.link(SYS_EXIT, spec.exitLabel or '结束'))
    handler[SYS_ENTER] = function(uid, value)
        dialogue.post(uid, spec.greet, menu)
    end

    for tag, callback in pairs(spec.extra or {}) do
        handler[tag] = callback
    end
    for tag, callback in pairs(handler) do
        if type(callback) == 'function' and tag ~= SYS_LABEL and tag ~= SYS_HIDE and tag ~= SYS_CHECKACTIVE and tag ~= SYS_ALLOWREDNAME then
            handler[tag] = dialogue.guardRedName(callback, spec.redName, spec.redNameExit or '结束')
        end
    end
    handler[SYS_ALLOWREDNAME] = true
    setEventHandler(handler)
    return handler
end

return grocer
