local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 布店 / 服装店 / 鞋店, legacy Market_Def/03Armor_*.txt (15) and 03Shoes_*.txt (2)
--
-- buys, sells and repairs, and unlike a smith never offers 特殊修理 — none of the 17 do
--
--     local outfitter = require('npc.include.merchant.outfitter')
--     outfitter.setOutfitter
--     {
--         greet = {'欢迎光临，需要什么？'},
--         goods = {'布衣（男）', '布衣（女）'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local outfitter = {}

function outfitter.setOutfitter(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty outfitter greeting')

    local label = spec.label or '防御工具'
    local trade = optList(spec.trade, {'衣服', '头盔'})
    local repair = optList(spec.repair, {'衣服', '头盔'})
    local repairDoneBack = spec.repairDoneBack
    if repairDoneBack == nil then repairDoneBack = spec.backLabel end
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '购买', spec.buySuffix or label))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'你要买什么？'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', spec.sellSuffix or label))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'请把要卖的衣服(头盔)放到上面。'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
    end

    if repair then
        table.insert(menu, dialogue.link(spec.preRepairText and 'npc_pre_repair' or 'npc_repair',
            spec.repairLabel or '修理', spec.repairSuffix or label))

        if spec.preRepairText then
            handler.npc_pre_repair = function(uid, value)
                dialogue.post(uid, spec.preRepairText, {dialogue.link('npc_repair', '修理')})
            end
        end
        handler.npc_repair = function(uid, value)
            dialogue.post(uid, spec.repairText or {'防御工具，头盔和帽子都可以修理。'}, back)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair, spec.repairDone, repairDoneBack)
        end
    end

    for _, topic in ipairs(spec.topics or {}) do
        table.insert(menu, dialogue.link(topic.id, topic.label, topic.suffix, topic.prefix))
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

return outfitter
