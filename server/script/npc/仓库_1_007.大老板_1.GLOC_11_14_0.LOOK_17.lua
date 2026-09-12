local dialog = require('include.dialog')
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
            dialog.post(uid, '我不愿意和你这样丧尽天良的人进行交易。',
            dialog.link(SYS_EXIT, '关闭'))
        else
            dialog.post(uid,
            {
                string.format('这里是沙巴克城<t color="RED">%s</t>行会的领地。', getSubukGuildName()),
                '这里寄存和出售道馆里使用的东西。',
            },
            {
                dialog.link('npc_goto_buy', '购买', {suffix = '物品'}),
                dialog.link('npc_goto_sell', '出售', {suffix = '物品'}),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_buy"] = function(uid, value)
        dialog.post(uid, '有什么需要的尽管挑。',
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_sell"] = function(uid, value)
        dialog.post(uid, '请把不用的东西卖给我，我给你个合理的价钱。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", goodsTypeList, tradeGold)
    end,
})
