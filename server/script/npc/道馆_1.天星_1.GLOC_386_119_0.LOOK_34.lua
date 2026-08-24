local invop = require('npc.include.invop')

local materialTypeList = {'道具'}
local tradeGold = 30

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
                    <par>欢迎光临，我收购蚂蚁卵或者骷髅骨之类的材料。</par>
                    <par></par>

                    <par><event id="npc_goto_1">出售</event>材料</par>
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
                <par>你要出售什么？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList, tradeGold)
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
