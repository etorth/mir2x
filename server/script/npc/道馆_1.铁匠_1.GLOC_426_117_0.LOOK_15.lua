setNPCSell({
    '木剑',
    '匕首',
    '青铜剑',
    '铁剑',
    '乌木剑',
    '半月',
})

local invop = require('npc.include.invop')

-- there is no item price in the item record yet, pay a flat price
-- keep query and commit on the same number, otherwise the quote lies to player
local tradeGold = 200

setEventHandler(
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
            uidPostXML(uid,
            [[
                <layout>
                    <par>这里是沙巴克城<t color="RED">%s</t>行会的领地。</par>
                    <par>这里是道馆寄存武器的地方，你需要什么武器吗？</par>
                    <par></par>

                    <par><event id="npc_goto_purchase">购买</event>武器</par>
                    <par><event id="npc_goto_trade">出售</event>武器</par>
                    <par><event id="npc_goto_repair">修理</event>武器</par>
                    <par><event id="npc_goto_special_repair">特殊修理</event>武器</par>
                    <par><event id="npc_goto_daily_quest">对今日的任务进行了解</event></par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_purchase"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>各种武器在这里保存得很好。</par>
                <par>你想要买什么武器？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        uidPostSell(uid)
    end,

    ["npc_goto_trade"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你有想卖掉的武器？</par>
                <par>让我看看。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", {'武器'})
    end,

    ["npc_goto_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请选择要修理的武器，我会报价。</par>
                <par>普通修理会有概率损失装备的持久上限。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'})
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>特殊修理不会损失持久上限，但是价钱要贵得多。</par>
                <par>请选择要修理的武器。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", {'武器'})
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        local itemID, seqID = invop.parseItemString(value)

        uidPostXML(uid,
        [[
            <layout>
                <par>你的武器%s太旧了，卖不了多少钱，报价%d金币。</par>
                <par>你要卖吗？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), tradeGold, SYS_ENTER)

        invop.postTradePrice(uid, itemID, seqID, tradeGold)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", {'武器'})
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        local itemID, seqID = invop.parseItemString(value)

        if uidRemove(uid, {itemID = itemID, seqID = seqID}) then
            uidGrantGold(uid, tradeGold)
            uidPostXML(uid,
            [[
                <layout>
                    <par>成交！支付你%d金币。</par>
                    <par></par>

                    <par><event id="%s">前一步</event></par>
                </layout>
            ]], tradeGold, SYS_ENTER)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>你的%s已经不在身上了。</par>
                    <par></par>

                    <par><event id="%s">前一步</event></par>
                </layout>
            ]], getItemName(itemID), SYS_ENTER)
        end

        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", {'武器'})
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'})
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'})
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", {'武器'})
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", {'武器'})
    end,
})
