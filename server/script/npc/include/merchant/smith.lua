local dialogue = require('npc.include.dialog')
local invop = require('npc.include.invop')

-- 铁匠 / 武器商, legacy Market_Def/02Weapon_*.txt (22 of them)
--
-- the only trade that offers 特殊修理 — 18 of the 22 do — and the only one with a word to say
-- about where good weapons come from (询问 -> @qweapon). two of them also take a bound weapon
-- off your hand (请求把剑从手分离开 -> @remove_sword), which is how a player gets rid of the
-- 攻杀铁剑 that 攻杀剑术任务 lends out
--
--     local smith = require('npc.include.merchant.smith')
--     smith.setSmith
--     {
--         greet = {'来点什么？我这儿的兵器都是好东西。'},
--         goods = {'青铜斧', '八荒', '凌风'},
--         special = false,                  -- 4 of the 22 do not offer 特殊修理
--         qweapon = true,                   -- offer the 询问 topic with the standard answer
--         removeSword = true,               -- offer 请求把剑从手分离开
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local smith = {}

-- @NPC_QWeapon, the same answer in every shop that has it
smith.QWEAPON =
{
    '商店里出售的武器基本上都差不多，但从怪物那里抢来的武器则根据不同的情况，可能具有超凡的能力。如果你把那类武器拿到商店里来卖，我可以出个好价钱。还有，武器的价格随着种类的不同而不同，但基本上持久性越强，价格就越高。',
}

-- @NPC_Remove_Sword and @NPC_Remove_Sword_Else. legacy checks what is in your hand first and
-- only claims to have unstuck the sword when there is one — three shops carry this and all three
-- name the same two swords, 攻杀铁剑 from 攻杀剑术任务 and 焱火剑 from 大火球任务, which are
-- the items SDItem::EA_BIND is actually used on
--
-- @NPC_Remove_Sword_1 is a byte-for-byte duplicate of @NPC_Remove_Sword in every one of the
-- three, so there is only one line to say
smith.REMOVE_SWORD =
{
    '你是怎么会让手粘在剑上呢 ...',
    '你看看现在是不是已经摘下来了... 这种没用的剑我来替你保管吧...',
}

smith.REMOVE_SWORD_ELSE =
{
    '你的手没有粘在剑上...',
    '听说<t color="red">攻杀铁剑</t>和 <t color="red">焱火剑</t>一旦到手上就摘不下来。',
}

-- the swords that stick. a smith will only take one of these off, so a player holding an
-- ordinary weapon does not get it confiscated by clicking the wrong menu entry
smith.BOUND_SWORD = {'攻杀铁剑', '焱火剑'}

function smith.setSmith(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty smith greeting')

    local label = spec.label or '武器'
    local trade = optList(spec.trade, {'武器', '矿石'})
    local repair = optList(spec.repair, {'武器'})
    local repairDoneBack = spec.repairDoneBack
    if repairDoneBack == nil then repairDoneBack = spec.backLabel end
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '购买', {suffix = spec.buySuffix or label}))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'请选择要购买的武器。'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', {suffix = spec.sellSuffix or label}))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'请把要卖的武器抬上来。'}, back)
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
            spec.repairLabel or '修理', {suffix = spec.repairSuffix or label}))

        if spec.preRepairText then
            handler.npc_pre_repair = function(uid, value)
                dialogue.post(uid, spec.preRepairText, {dialogue.link('npc_repair', '修理')})
            end
        end
        handler.npc_repair = function(uid, value)
            dialogue.post(uid, spec.repairText or {'请把要修理的武器放上去。'}, back)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair, spec.repairDone, repairDoneBack)
        end
    end

    -- Some smiths offer only special repair, without an ordinary repair counter.
    if spec.special ~= false then
        local specialRepair = spec.specialRepair or repair or {'武器'}
        table.insert(menu, dialogue.link('npc_special_repair', spec.specialLabel or '特殊修理', {suffix = spec.specialSuffix or label}))
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

    if spec.qweapon then
        table.insert(menu, dialogue.link('npc_qweapon', '询问', {suffix = spec.qweaponSuffix or '关于武器的事'}))
        handler.npc_qweapon = function(uid, value)
            dialogue.post(uid, spec.qweaponText or smith.QWEAPON, back)
        end
    end

    if spec.removeSword then
        table.insert(menu, dialogue.link('npc_remove_sword', '请求把剑从手分离开'))
        handler.npc_remove_sword = function(uid, value)
            local held = server.player.getWLItem(uid, WLG_WEAPON)
            if held then
                for _, name in ipairs(smith.BOUND_SWORD) do
                    if held.itemID == getItemID(name) and server.player.removeWearItem(uid, WLG_WEAPON) then
                        dialogue.post(uid, spec.removeSwordText or smith.REMOVE_SWORD, back)
                        return
                    end
                end
            end
            dialogue.post(uid, spec.removeSwordElseText or smith.REMOVE_SWORD_ELSE, back)
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

    if spec.exitLabel ~= false then
        table.insert(menu, dialogue.link(SYS_EXIT, spec.exitLabel or '结束'))
    end
    handler[SYS_ENTER] = function(uid, value)
        if spec.onEnter and spec.onEnter(uid, value) then
            return
        end
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

return smith
