local invop = require('npc.include.invop')

-- a standard merchant, the shape almost every Market_Def script in the legacy scripts uses
--
-- an NPC only describes what it deals in, the menu and every handler get generated here:
--
--     local shop = require('npc.include.shop')
--     shop.setMerchant{
--         greet  = {'欢迎光临，有什么事吗？'},
--         goods  = {'青铜头盔', '魔法头盔'},          -- nil when it does not sell
--         label  = '防御工具',                        -- what the menu calls the goods
--         trade  = {'衣服', '头盔'},                  -- item types it buys back, nil when it does not
--         repair = {'衣服', '头盔'},                  -- item types it repairs, nil when it does not
--         special = true,                             -- also offer 特殊修理
--         price  = 50,                                -- buyback price, see invop.postQueryTrade()
--         extra  = {...},                             -- extra handlers merged into the table
--     }

local shop = {}

local function parList(textList)
    local out = {}
    for _, text in ipairs(textList) do
        table.insert(out, string.format('                        <par>%s</par>', text))
    end
    return table.concat(out, '\n')
end

function shop.setMerchant(arg)
    local label = arg.label or '物品'
    local price = arg.price or 50

    local menu = {}
    if arg.goods  then table.insert(menu, string.format('<par><event id="npc_goto_purchase">购买</event>%s</par>', label)) end
    if arg.trade  then table.insert(menu, string.format('<par><event id="npc_goto_trade">出售</event>%s</par>', label)) end
    if arg.repair then
        table.insert(menu, string.format('<par><event id="npc_goto_repair">修理</event>%s</par>', label))
        if arg.special then
            table.insert(menu, string.format('<par><event id="npc_goto_special_repair">特殊修理</event>%s</par>', label))
        end
    end

    for _, entry in ipairs(arg.menu or {}) do
        table.insert(menu, string.format('<par><event id="%s">%s</event>%s</par>', entry.id, entry.label, entry.suffix or ''))
    end

    if arg.goods then
        setNPCSell(arg.goods)
    end

    local handler =
    {
        [SYS_ENTER] = function(uid, value)
            if uidQueryRedName(uid) then
                uidPostXML(uid,
                [[
                    <layout>
                        <par>我不愿意和你这样丧尽天良的人进行交易。</par>
                        <par></par>

                        <par><event id="%s" close="1">关闭</event></par>
                    </layout>
                ]], SYS_EXIT)
            else
                uidPostXML(uid, string.format(
                [[
                    <layout>
%s
                        <par></par>

%s
                        <par><event id="%%s" close="1">关闭</event></par>
                    </layout>
                ]], parList(arg.greet or {'欢迎光临。'}), '                        '..table.concat(menu, '\n                        ')), SYS_EXIT)
            end
        end,
    }

    if arg.goods then
        handler["npc_goto_purchase"] = function(uid, value)
            uidPostXML(uid, string.format(
            [[
                <layout>
%s
                    <par></par>

                    <par><event id="%%s">前一步</event></par>
                </layout>
            ]], parList(arg.buyText or {string.format('请选择要购买的%s。', label)})), SYS_ENTER)
            uidPostSell(uid)
        end
    end

    if arg.trade then
        handler["npc_goto_trade"] = function(uid, value)
            uidPostXML(uid, string.format(
            [[
                <layout>
%s
                    <par></par>

                    <par><event id="%%s">前一步</event></par>
                </layout>
            ]], parList(arg.tradeText or {string.format('请把要出售的%s拿上来。', label)})), SYS_ENTER)
            invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", arg.trade)
        end

        handler["npc_goto_query_trade"] = function(uid, value)
            invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", arg.trade, price)
        end

        handler["npc_goto_commit_trade"] = function(uid, value)
            invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", arg.trade, price)
        end
    end

    if arg.repair then
        handler["npc_goto_repair"] = function(uid, value)
            uidPostXML(uid, string.format(
            [[
                <layout>
%s
                    <par>普通修理会有概率损失装备的持久上限。</par>
                    <par></par>

                    <par><event id="%%s">前一步</event></par>
                </layout>
            ]], parList(arg.repairText or {string.format('请把要修理的%s放上来。', label)})), SYS_ENTER)
            invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", arg.repair)
        end

        handler["npc_goto_query_repair"] = function(uid, value)
            invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", arg.repair)
        end

        handler["npc_goto_commit_repair"] = function(uid, value)
            invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", arg.repair)
        end

        if arg.special then
            handler["npc_goto_special_repair"] = function(uid, value)
                uidPostXML(uid,
                [[
                    <layout>
                        <par>特殊修理不会损失持久上限，但是价钱要贵得多。</par>
                        <par></par>

                        <par><event id="%s">前一步</event></par>
                    </layout>
                ]], SYS_ENTER)
                invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", arg.repair)
            end

            handler["npc_goto_query_special_repair"] = function(uid, value)
                invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", arg.repair)
            end

            handler["npc_goto_commit_special_repair"] = function(uid, value)
                invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", arg.repair)
            end
        end
    end

    for tag, func in pairs(arg.extra or {}) do
        handler[tag] = func
    end

    setEventHandler(handler)
    return handler
end

return shop
