local dialog = require('include.dialog')
local invop = require('npc.include.invop')

local weaponTypeList = {'武器'}

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            dialog.post(uid, '我不想和你这种坏人做生意。',
            dialog.link(SYS_EXIT, '关闭'))
        else
            dialog.post(uid, '欢迎光临，有什么事吗？',
            {
                dialog.link('npc_goto_1', '修理', {suffix = '武器'}),
                dialog.link('npc_goto_2', '特殊修理', {suffix = '武器'}),
                dialog.link('npc_goto_3', '对今日的任务进行了解'),
                dialog.link(getSubukGuildName(), '关闭', {close = true}),
            })
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        dialog.post(uid,
        {
            '请把要修理的武器拿来。',
            '普通修理会有概率损失武器的持久上限。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_2"] = function(uid, value)
        dialog.post(uid,
        {
            '特殊修理不会损失持久上限，价钱贵些。',
            '请把要修理的武器拿来。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_3"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
