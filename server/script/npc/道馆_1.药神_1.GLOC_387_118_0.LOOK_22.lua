setNPCSell({
    '黄色药粉（小）',
    '黄色药粉（中）',
    '黄色药粉（大）',
    '灰色药粉（小）',
    '灰色药粉（中）',
    '灰色药粉（大）',
})

local invop = require('npc.include.invop')

local powderTypeList = {'药粉'}
local tradeGold = 20

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
                    <par>嗯，我在这里进行试验，制造各类药粉，同时也传授知识。你需要什么？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>药粉</par>
                    <par><event id="npc_goto_trade">出售</event>药粉</par>
                    <par><event id="npc_goto_2">对今日的任务进行了解</event></par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请选择你想购买的药粉。</par>
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
                <par>多余的药粉我也收，放上来吧。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList, tradeGold)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
