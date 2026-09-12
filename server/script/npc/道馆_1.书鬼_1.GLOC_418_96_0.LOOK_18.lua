local dialog = require('include.dialog')
setNPCSell({
    '治愈术',
})

local invop = require('npc.include.invop')

local bookTypeList = {'技能书'}
local tradeGold = 50

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
                '欢迎光临，你来买练武功的技能书籍？',
            },
            {
                dialog.link('npc_goto_1', '购买', {suffix = '图书'}),
                dialog.link('npc_goto_2', '出售', {suffix = '图书'}),
                dialog.link('npc_goto_3', '聆听', {suffix = '关于技能书籍的说明'}),
                dialog.link('npc_goto_4', '对今日的任务进行了解'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        dialog.post(uid, '请挑选你想要的书。',
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_2"] = function(uid, value)
        dialog.post(uid, '请把要出售的书籍拿上来。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", bookTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", bookTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", bookTypeList, tradeGold)
    end,

    ["npc_goto_3"] = function(uid, value)
        dialog.post(uid,
        {
            '你想听哪类书的介绍？',
            '道士可以学习' .. dialog.link('npc_goto_5', '治愈术') .. '，' .. dialog.link('npc_goto_6', '精神力战法') .. '和' .. dialog.link('npc_goto_7', '施毒术') .. '。',
        },
        dialog.link(SYS_ENTER, '前一步'))
    end,

    ["npc_goto_4"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,

    ["npc_goto_5"] = function(uid, value)
        dialog.post(uid, '等级为7时可以修炼<t color="RED">治愈术</t>的第1阶段，等级为11时修炼第2阶段，16级时可以完成第3阶段的修炼。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    ["npc_goto_6"] = function(uid, value)
        dialog.post(uid, '等级为8时可以修炼<t color="RED">精神力战法</t>的第1阶段，等级为10时修炼第2阶段，12级时可以完成第3阶段的修炼。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    ["npc_goto_7"] = function(uid, value)
        dialog.post(uid, '等级为12时可以修炼<t color="RED">施毒术</t>的第1阶段，等级为14时修炼第2阶段，16级时可以完成第3阶段的修炼。',
        dialog.link(SYS_ENTER, '前一步'))
    end,
})
