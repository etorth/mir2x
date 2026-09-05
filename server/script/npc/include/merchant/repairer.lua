local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 修理店, legacy Market_Def/09Repair_*.txt (2)
--
-- mends things and trades in nothing. no [Goods], no @buy, no @sell — the mirror image of
-- npc/include/merchant/buyer.lua
--
--     local repairer = require('npc.include.merchant.repairer')
--     repairer.setRepairer
--     {
--         greet = {'要修理什么？'},
--         repair = {'武器', '衣服', '头盔'},
--         special = true,
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local repairer = {}

function repairer.setRepairer(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty repairer greeting')

    local label = spec.label or '装备'
    local repair = optList(spec.repair, {'武器', '衣服', '头盔'})
    local repairDoneBack = spec.repairDoneBack
    if repairDoneBack == nil then repairDoneBack = spec.backLabel end
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if repair then
        table.insert(menu, dialogue.link('npc_repair', spec.repairLabel or '修理', spec.repairSuffix or label))
        handler.npc_repair = function(uid, value)
            dialogue.post(uid, spec.repairText or {'请把要修理的装备放上来。'}, back)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair, spec.repairDone, repairDoneBack)
        end
    end

    if spec.special then
        local specialRepair = spec.specialRepair or repair
        assertType(specialRepair, 'table')
        table.insert(menu, dialogue.link('npc_special_repair', spec.specialLabel or '特殊修理', spec.specialSuffix or label))
        handler.npc_special_repair = function(uid, value)
            dialogue.post(uid, spec.specialText or {'特殊修理不会损失持久上限，但是价钱要贵得多。'}, back)
            invop.uidStartRepair(uid, 'npc_special_query', 'npc_special_commit', specialRepair)
        end
        handler.npc_special_query = function(uid, value)
            invop.postQuerySpecialRepair(uid, value, 'npc_special_query', 'npc_special_commit', specialRepair)
        end
        handler.npc_special_commit = function(uid, value)
            invop.postCommitSpecialRepair(uid, value, 'npc_special_query', 'npc_special_commit', specialRepair, spec.repairDone, repairDoneBack)
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

return repairer
