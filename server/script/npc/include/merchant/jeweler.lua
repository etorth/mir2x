local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 饰品店, legacy Market_Def/08Accessory_*.txt (12) and 08Astrologist_*.txt (2)
--
-- rings, bracelets and necklaces. buys, sells and repairs, no 特殊修理. the shop that tells you
-- to check durability before you commit — 请先看好价钱和持久性再决定
--
--     local jeweler = require('npc.include.merchant.jeweler')
--     jeweler.setJeweler
--     {
--         greet = {'你想买饰品?'},
--         goods = {'铁手镯', '银戒指'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local jeweler = {}

-- QuestDiary/Make_Item/AtomItem.txt chooses the first available item in this order.
jeweler.RUST_ACCESSORIES =
{
    '师承戒指', '龙马戒指', '青云戒指',
    '破荒项链', '魔云项链', '定心项链',
    '金棱手镯', '思过手镯', '世尊手镯',
}
jeweler.RUST_ELEMENTS =
{
    {name = '火',   tag = 'npc_rust_fire'},
    {name = '冰',   tag = 'npc_rust_ice'},
    {name = '雷',   tag = 'npc_rust_light'},
    {name = '风',   tag = 'npc_rust_wind'},
    {name = '神圣', tag = 'npc_rust_holy'},
    {name = '暗黑', tag = 'npc_rust_dark'},
    {name = '幻影', tag = 'npc_rust_phantom'},
}
jeweler.RUST_PRICE = 1000000

function jeweler.setJeweler(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty jeweler greeting')

    local label = spec.label or '饰品'
    local trade = optList(spec.trade, {'手镯', '戒指', '项链'})
    local repair = optList(spec.repair, {'手镯', '戒指', '项链'})
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
            dialogue.post(uid, spec.buyText or {'你想买饰品? 想买什么？请先看好价钱和持久性再决定。'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', spec.sellSuffix or label))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'你想出售饰品？', '请先把东西拿出来给我看看。'}, back)
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
        table.insert(menu, dialogue.link('npc_repair', spec.repairLabel or '修理', spec.repairSuffix or label))
        handler.npc_repair = function(uid, value)
            dialogue.post(uid, spec.repairText or {'你想修理饰品?'}, back)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair)
        end
        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', repair, spec.repairDone, repairDoneBack)
        end
    end

    if spec.rustAccessory then
        local oldItemIDs = {}
        for _, base in ipairs(jeweler.RUST_ACCESSORIES) do
            local itemID = getItemID('生锈' .. base)
            assert(itemID > 0, 'unknown rusty accessory: ' .. base)
            table.insert(oldItemIDs, itemID)
        end

        local close = {dialogue.link(SYS_EXIT, '关闭')}
        local function postNoMaterial(uid)
            dialogue.post(uid, {'你弄错了吧,这不是古代勇士们使用过的生锈饰品,得到它们之后再来找我吧.'}, close)
        end

        local elements = {}
        for _, element in ipairs(jeweler.RUST_ELEMENTS) do
            local rewardIDs = {}
            for _, base in ipairs(jeweler.RUST_ACCESSORIES) do
                local itemID = getItemID(base .. '（' .. element.name .. '）')
                assert(itemID > 0, 'unknown elemental accessory: ' .. base .. element.name)
                table.insert(rewardIDs, itemID)
            end
            table.insert(elements, dialogue.link(element.tag, element.name .. '元素.'))
            handler[element.tag] = function(uid, value)
                -- Keep the checks and exchange in one player-side call, including stale clicks.
                local restored = uidRemoteCall(uid, oldItemIDs, rewardIDs, jeweler.RUST_PRICE,
                [[
                    local oldItemIDs, rewardIDs, price = ...
                    if getGold() < price then
                        return 0
                    end
                    for index, itemID in ipairs(oldItemIDs) do
                        if hasItem(itemID, 0, 1) then
                            if not removeItem(itemID, 0, 1) then
                                return 0
                            end
                            if not removeGold(price) then
                                addItem(itemID, 1)
                                return 0
                            end
                            addItem(rewardIDs[index], 1)
                            return index
                        end
                    end
                    return 0
                ]])
                assertType(restored, 'integer')
                if restored == 0 then
                    postNoMaterial(uid)
                elseif restored == 9 and element.name == '暗黑' then
                    -- This reachable branch uses inline text, not the clean but unused include.
                    dialogue.post(uid, {'世尊手镯（暗黑）捞 咯扁 乐嚼聪促.'})
                else
                    dialogue.post(uid, {string.format('得到%s(%s).', jeweler.RUST_ACCESSORIES[restored], element.name)}, close)
                end
            end
        end
        table.insert(elements, dialogue.link(SYS_EXIT, '再想一想.'))

        table.insert(menu, dialogue.link('npc_rustaccessory', '询问生锈饰品.'))
        handler.npc_rustaccessory = function(uid, value)
            dialogue.post(uid,
            {
                '村庄附近的诺玛遗址里经常出现古代勇士们使用过的元素饰品. 有些饰品因为生锈而失去了原有的功能.',
                '但不要小瞧它们,更不要乱丢.用诺玛族秘传的方法可以让它们恢复原貌.',
                '不过,如果你想让手中生锈的饰品恢复原貌,就要支付一定费用. 嗯,对了,你还可以反复更换复原饰品的攻击元素,不过只有耐久完好的饰品才可以变换攻击元素.',
            },
            {
                dialogue.link('npc_rust_restore', '支付100万金币,将生锈的饰品恢复原貌.'),
                dialogue.link('npc_rust_help', '讯问元素道具.'),
                dialogue.link(SYS_EXIT, '关闭.'),
            })
        end
        handler.npc_rust_restore = function(uid, value)
            local status = uidRemoteCall(uid, oldItemIDs, jeweler.RUST_PRICE,
            [[
                local oldItemIDs, price = ...
                if getGold() < price then
                    return -1
                end
                for _, itemID in ipairs(oldItemIDs) do
                    if hasItem(itemID, 0, 1) then
                        return 1
                    end
                end
                return 0
            ]])
            assertType(status, 'integer')
            if status == -1 then
                dialogue.post(uid, {'因为金币不够,所以不能帮你恢复饰品的原貌.'}, close)
            elseif status == 0 then
                postNoMaterial(uid)
            else
                dialogue.post(uid, {'想恢复生锈饰品的哪一种攻击元素属性呢?'}, elements)
            end
        end
        handler.npc_rust_help = function(uid, value)
            dialogue.post(uid,
            {
                '我所能恢复的饰品有 <t color="red">生锈的师承戒指, 生锈的龙马戒指, 生锈的青云戒指, 生锈的破荒项链, 生锈的魔云项链, 生锈的定心项链, 生锈的金棱手镯, 生锈的思过手镯, 生锈的世尊手镯</t>. 这些都是古代勇士曾经使用过的饰品,只要你支付一定费用,我会帮你恢复饰品的原貌.',
                '恢复原貌的饰品,还可以反复地更换攻击元素.',
            }, close)
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

return jeweler
