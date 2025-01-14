setNPCSell({
    '木剑',
    '匕首',
    '青铜剑',
    '铁剑',
    '乌木剑',
    '半月',
})

local invop = require('npc.include.invop')
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
                <par></par>

                <par><event id="npc_goto_6">修理</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'})
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修理费用是%d。</par>
                <par>这个特修是只有带着物品的情况下才可以修理。请确认一下是否带着。</par>
                <par></par>

                <par><event id="npc_goto_commit_special_repair">修理</event></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], 0, SYS_ENTER)
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
        itemID, seqID = invop.parseItemString(value)
        price = math.random(100, 200)

        uidPostXML(uid,
        [[
            <layout>
                <par>你的武器%s太旧了，卖不了多少钱，报价%s金币。</par>
                <par>你要卖吗？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), price, SYS_ENTER)
        invop.postTradePrice(uid, itemID, seqID, price)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        itemID, seqID = invop.parseItemString(value)
        uidPostXML(uid,
        [[
            <layout>
                <par>成交！支付你你%d金币。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], 200, SYS_ENTER)

        uidRemove(uid, {itemID = itemID, seqID = seqID})
        uidGrantGold(uid, 200)
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        itemID, seqID = invop.parseItemString(value)
        repairCost = math.random(100, 200)

        uidPostXML(uid,
        [[
            <layout>
                <par>你的武器%s太旧了，修复费用是%s金币。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), repairCost, SYS_ENTER)
        invop.postRepairCost(uid, itemID, seqID, repairCost)
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        itemID, seqID = invop.parseItemString(value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你的%s已经修理完毕。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), SYS_ENTER)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修理得很好。</par>
                <par>修理费用是%d金币。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], 0, SYS_ENTER)
    end,
})
