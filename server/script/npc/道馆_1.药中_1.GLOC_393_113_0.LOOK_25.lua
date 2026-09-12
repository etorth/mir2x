local dialog = require('include.dialog')
setNPCSell({
    '金创药（小）',
    '魔法药（小）',
    '金创药（中）',
    '魔法药（中）',
    '金创药（大）',
    '魔法药（大）',
    '金创药（特）',
    '魔法药（特）',
    '太阳水',
})

local invop = require('npc.include.invop')

local potionTypeList = {'恢复药水'}
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
                '我是在这里学习药学的研修生，你需要什么东西？',
            },
            {
                dialog.link('npc_goto_1', '购买', {suffix = '药品'}),
                dialog.link('npc_goto_2', '出售', {suffix = '药品'}),
                dialog.link('npc_goto_3', '对今日的任务进行了解'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        dialog.post(uid, '请挑选你所需要的药和用量。',
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_2"] = function(uid, value)
        dialog.post(uid, '请把你想出售的药放在这里。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", potionTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", potionTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", potionTypeList, tradeGold)
    end,

    ["npc_goto_3"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
