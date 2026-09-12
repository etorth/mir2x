local dialog = require('include.dialog')
setNPCSell({
    '木剑',
    '匕首',
    '青铜剑',
    '铁剑',
    '乌木剑',
    '半月',
})

local invop = require('npc.include.invop')

-- there is no item price in the item record yet, pay a flat price
-- keep query and commit on the same number, otherwise the quote lies to player
local weaponTypeList = {'武器'}
local tradeGold = 200

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
                '这里是道馆寄存武器的地方，你需要什么武器吗？',
            },
            {
                dialog.link('npc_goto_purchase', '购买', {suffix = '武器'}),
                dialog.link('npc_goto_trade', '出售', {suffix = '武器'}),
                dialog.link('npc_goto_repair', '修理', {suffix = '武器'}),
                dialog.link('npc_goto_special_repair', '特殊修理', {suffix = '武器'}),
                dialog.link('npc_goto_daily_quest', '对今日的任务进行了解'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_purchase"] = function(uid, value)
        dialog.post(uid,
        {
            '各种武器在这里保存得很好。',
            '你想要买什么武器？',
        },
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_trade"] = function(uid, value)
        dialog.post(uid,
        {
            '你有想卖掉的武器？',
            '让我看看。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", weaponTypeList)
    end,

    ["npc_goto_repair"] = function(uid, value)
        dialog.post(uid,
        {
            '请选择要修理的武器，我会报价。',
            '普通修理会有概率损失装备的持久上限。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        dialog.post(uid,
        {
            '特殊修理不会损失持久上限，但是价钱要贵得多。',
            '请选择要修理的武器。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", weaponTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", weaponTypeList, tradeGold)
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
})
