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
            dialog.post(uid, '很高兴见到你，有什么事吗？',
            {
                dialog.link('npc_goto_special_repair', '特殊修理', {suffix = '武器'}),
                dialog.link('npc_goto_unequip_weapon', '请求把剑从手分离开'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        dialog.post(uid,
        {
            '特殊修理不会损失武器的持久上限，价钱要贵得多。',
            '请选择要修理的武器。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_unequip_weapon"] = function(uid, value)
        dialog.post(uid, '我不会。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
