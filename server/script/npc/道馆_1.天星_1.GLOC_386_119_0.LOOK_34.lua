local dialog = require('include.dialog')
local invop = require('npc.include.invop')

local materialTypeList = {'道具'}
local tradeGold = 30

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
                '欢迎光临，我收购蚂蚁卵或者骷髅骨之类的材料。',
            },
            {
                dialog.link('npc_goto_1', '出售', {suffix = '材料'}),
                dialog.link('npc_goto_2', '对今日的任务进行了解'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        dialog.post(uid, '你要出售什么？',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", materialTypeList, tradeGold)
    end,

    ["npc_goto_2"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
