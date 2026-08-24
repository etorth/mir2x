setNPCSell({
    '青铜头盔',
    '魔法头盔',
    '布衣（男）',
    '轻盔甲（男）',
    '布衣（女）',
    '轻盔甲（女）',
})

local invop = require('npc.include.invop')

local armourTypeList = {'衣服', '头盔'}
local tradeGold = 50

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
                    <par>你需要什么？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>防御工具</par>
                    <par><event id="npc_goto_2">出售</event>防御工具</par>
                    <par><event id="npc_goto_3">修理</event>防御工具</par>
                    <par><event id="npc_goto_4">对今日的任务进行了解</event></par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>来，需要吗？挑一下吧。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        uidPostSell(uid)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要卖的衣服（头盔）放到上面。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", armourTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", armourTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", armourTypeList, tradeGold)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>确实要修理吗？</par>
                <par>普通修理会有概率损失持久上限，特殊修理不会。</par>
                <par></par>

                <par><event id="npc_goto_5">修理</event></par>
                <par><event id="npc_goto_special_repair">特殊修理</event></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>特殊修理不会损失持久上限，价钱贵些。</par>
                <par>请把要修理的衣服（头盔）放上来。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", armourTypeList)
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", armourTypeList)
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", armourTypeList)
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", armourTypeList)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", armourTypeList)
    end,

    ["npc_goto_4"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,

    ["npc_goto_5"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要修理的衣服（头盔）放上来，嗯，东西弄得很脏啊。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", armourTypeList)
    end,
})
