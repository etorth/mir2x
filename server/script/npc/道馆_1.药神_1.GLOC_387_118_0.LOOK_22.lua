local dialog = require('include.dialog')
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
            dialog.post(uid, '我不愿意和你这样丧尽天良的人进行交易。',
            dialog.link(SYS_EXIT, '关闭'))
        else
            dialog.post(uid,
            {
                string.format('这里是沙巴克城<t color="RED">%s</t>行会的领地。', getSubukGuildName()),
                '嗯，我在这里进行试验，制造各类药粉，同时也传授知识。你需要什么？',
            },
            {
                dialog.link('npc_goto_1', '购买', {suffix = '药粉'}),
                dialog.link('npc_goto_trade', '出售', {suffix = '药粉'}),
                dialog.link('npc_goto_2', '对今日的任务进行了解'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        dialog.post(uid, '请选择你想购买的药粉。',
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_trade"] = function(uid, value)
        dialog.post(uid, '多余的药粉我也收，放上来吧。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", powderTypeList, tradeGold)
    end,

    ["npc_goto_2"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
