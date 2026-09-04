local invop = require('npc.include.invop')

-- the mechanics behind a merchant, and nothing about what kind of merchant it is
--
-- an NPC never requires this directly. it requires one of npc/include/merchant/*.lua — smith,
-- outfitter, jeweler, apothecary, bookseller, grocer, butcher, buyer, repairer — and that typed
-- module fills in what its trade does and hands the result here
--
-- the split exists because the trades really do differ. from the legacy Market_Def data:
--
--   02Weapon  buy/sell/repair, and the only type with 特殊修理 (18 of 22), plus 询问 stock talk
--             and 请求把剑从手分离开 for a bound weapon
--   03Armor   buy/sell/repair, never 特殊修理
--   03Shoes   as 03Armor
--   08Access  buy/sell/repair
--   04Potion  buy/sell, never repairs anything (0 of 26)
--   05Book    buy/sell, never repairs
--   07Grocery buy/sell, one of 16 repairs
--   01Meet    mostly sell-only (5 of 8 do not buy), says 卖/买 rather than 出售/购买, and 12 of
--             them answer 询问获取肉的途径
--   10Materi  sell only — it buys from the player and stocks nothing (0 of 13 sell)
--   09Repair  repair only, no trade at all
--
-- everything a legacy script can say is overridable per NPC. that is deliberate: an audit of the
-- first conversion pass against Envir found 275 whole dialogue sections dropped because the old
-- single shop.lua had nowhere to put them — 77 red-name refusals in the NPC's own words, 47
-- repair prompts, 19 greetings, and every one of the 100 今日任务 blocks
--
--     spec fields a typed module sets, and an NPC may override:
--
--     greet       {...}    required, the @NPC_Main text. no default: a merchant with nothing to
--                          say is the bug this audit found 19 times
--     redName     '...'    what it says to a red name, in this NPC's own words
--     label       '...'    what the menu calls the goods, e.g. 武器 / 防御工具 / 药品
--     goods       {...}    what it stocks, from the legacy [Goods] section. nil = does not sell
--     buyLabel    '...'    購買 for most, 买 for a butcher
--     sellLabel   '...'    出售 for most, 卖 for a butcher
--     trade       {...}    item types it buys back. nil = does not buy
--     price       n        buyback price, see invop.postQueryTrade
--     repair      {...}    item types it repairs. nil = does not repair
--     special     bool     also offer 特殊修理
--     buyText     {...}    the @NPC_Buy prompt
--     sellText    {...}    the @NPC_Sell prompt
--     repairText  {...}    the @NPC_Repair or @NPC_Pre_Repair prompt
--     repairDone  '...'    the @NPC_Repair_Complete line, said once a repair succeeds
--     today       '...'    the @NPC_TQuest fallback. see the note on daily quests below
--     topics      {...}    sub-conversations, see below
--     extra       {...}    raw handlers merged in last, for anything one-off
--
-- topics carry the branches that are not buying or selling — 询问 on a smith, 询问获取肉的途径
-- on a butcher, 关于武功书的说明 on a bookseller, and the area-quest talk several NPCs have:
--
--     topics =
--     {
--         {id = 'npc_qweapon', label = '询问', suffix = '关于武器的事',
--          text = {'商店里出售的武器基本上都差不多，...'}},
--     }
--
-- a topic may instead carry handler = function(uid, value) ... end to do something.

local merchant = {}

-- every legacy merchant offers 对今日的任务进行了解 -> @TQuest. behind it is CheckDailyQuest,
-- which rotates a daily quest id and delegates into QuestDiary/QT_TODAY. mir2x has no daily
-- quest system, so the branches have nothing to hook to — but the fallback the player sees when
-- no daily quest is live is real text, and with no daily system that is the only answer there is
merchant.TODAY_LABEL = '对今日的任务进行了解'
merchant.TODAY_NONE   = '今天没事情可拜托你了。'

local function parList(textList, indent)
    local out = {}
    for _, text in ipairs(textList) do
        table.insert(out, string.format('%s<par>%s</par>', indent, text))
    end
    return table.concat(out, '\n')
end

-- a page of text with a way back to the menu
local function postPage(uid, textList, backLabel)
    uidPostXML(uid, string.format(
    [[
        <layout>
%s
            <par></par>
            <par><event id="%%s">%s</event></par>
        </layout>
    ]], parList(textList, '            '), backLabel or '前一步'), SYS_ENTER)
end

function merchant.setMerchant(spec)
    assertType(spec, 'table')

    -- the one thing with no sensible default
    assertType(spec.greet, 'table')
    if #spec.greet == 0 then
        fatalPrintf('merchant %s has an empty greet, put the legacy @NPC_Main text there', getNPCName())
    end

    local label     = spec.label or '物品'
    local buyLabel  = spec.buyLabel or '购买'
    local sellLabel = spec.sellLabel or '出售'
    local price     = spec.price or 50

    local menu = {}
    local handler = {}

    -- stored without the <par> wrapper, parList adds it
    local function addMenu(id, text, suffix)
        table.insert(menu, string.format('<event id="%s">%s</event>%s', id, text, suffix or ''))
    end

    -- 购买
    if spec.goods then
        setNPCSell(spec.goods)
        addMenu('npc_buy', buyLabel, label)

        handler.npc_buy = function(uid, value)
            postPage(uid, spec.buyText or {string.format('请选择要购买的%s。', label)})
            uidPostSell(uid)
        end
    end

    -- 出售
    if spec.trade then
        addMenu('npc_sell', sellLabel, label)

        handler.npc_sell = function(uid, value)
            postPage(uid, spec.sellText or {string.format('请把要出售的%s拿上来。', label)})
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', spec.trade)
        end

        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', spec.trade, price)
        end

        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', spec.trade, price)
        end
    end

    -- 修理 and 特殊修理
    if spec.repair then
        addMenu('npc_repair', '修理', label)

        handler.npc_repair = function(uid, value)
            local text = {}
            for _, line in ipairs(spec.repairText or {string.format('请把要修理的%s放上来。', label)}) do
                table.insert(text, line)
            end
            table.insert(text, '普通修理会有概率损失装备的持久上限。')

            postPage(uid, text)
            invop.uidStartRepair(uid, 'npc_repair_query', 'npc_repair_commit', spec.repair)
        end

        handler.npc_repair_query = function(uid, value)
            invop.postQueryRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', spec.repair)
        end

        handler.npc_repair_commit = function(uid, value)
            invop.postCommitRepair(uid, value, 'npc_repair_query', 'npc_repair_commit', spec.repair, spec.repairDone)
        end

        if spec.special then
            addMenu('npc_special_repair', '特殊修理', label)

            handler.npc_special_repair = function(uid, value)
                postPage(uid, spec.specialText or {'特殊修理不会损失持久上限，但是价钱要贵得多。'})
                invop.uidStartRepair(uid, 'npc_special_query', 'npc_special_commit', spec.repair)
            end

            handler.npc_special_query = function(uid, value)
                invop.postQuerySpecialRepair(uid, value, 'npc_special_query', 'npc_special_commit', spec.repair)
            end

            handler.npc_special_commit = function(uid, value)
                invop.postCommitSpecialRepair(uid, value, 'npc_special_query', 'npc_special_commit', spec.repair, spec.repairDone)
            end
        end
    end

    -- the topics: 询问, 询问获取肉的途径, the area-quest talk, whatever this NPC has
    for _, topic in ipairs(spec.topics or {}) do
        assertType(topic.id, 'string')
        assertType(topic.label, 'string')
        addMenu(topic.id, topic.label, topic.suffix)

        if topic.handler then
            handler[topic.id] = topic.handler
        else
            assertType(topic.text, 'table')
            local textList = topic.text
            local backLabel = topic.back
            handler[topic.id] = function(uid, value)
                postPage(uid, textList, backLabel)
            end
        end
    end

    -- 对今日的任务进行了解, last in the menu the way legacy has it
    addMenu('npc_today', merchant.TODAY_LABEL)
    handler.npc_today = function(uid, value)
        postPage(uid, {spec.today or merchant.TODAY_NONE})
    end

    handler[SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid, string.format(
            [[
                <layout>
                    <par>%s</par>
                    <par></par>
                    <par><event id="%%s" close="1">关闭</event></par>
                </layout>
            ]], spec.redName or '我不愿意和你这样丧尽天良的人进行交易。'), SYS_EXIT)
            return
        end

        uidPostXML(uid, string.format(
        [[
            <layout>
%s
                <par></par>
%s
                <par><event id="%%s" close="1">结束</event></par>
            </layout>
        ]], parList(spec.greet, '                '), parList(menu, '                ')), SYS_EXIT)
    end

    for tag, func in pairs(spec.extra or {}) do
        handler[tag] = func
    end

    setEventHandler(handler)
    return handler
end

return merchant
