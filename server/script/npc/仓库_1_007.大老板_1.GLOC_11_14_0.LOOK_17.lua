setNPCSell({
    '金创药（小）',
    '魔法药（小）',
    '回城卷',
    '随机传送卷',
})

local invop = require('npc.include.invop')

local goodsTypeList = {'恢复药水', '传送卷轴', '药粉', '道具'}
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
                    <par>这里寄存和出售道馆里使用的东西。</par>
                    <par></par>
                    <par><event id="npc_goto_buy" >购买</event>物品</par>
                    <par><event id="npc_goto_sell">出售</event>物品</par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_buy"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>有什么需要的尽管挑。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        uidPostSell(uid)
    end,

    ["npc_goto_sell"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把不用的东西卖给我，我给你个合理的价钱。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList, tradeGold)
    end,
})
