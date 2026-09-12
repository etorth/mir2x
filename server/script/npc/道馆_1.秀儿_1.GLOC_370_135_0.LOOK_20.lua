local dialog = require('include.dialog')
setNPCSell({
    '水晶魔戒',
    '珍珠戒指',
    '道士手镯',
    '黑色水晶项链',
    '凤凰明珠',
})

local invop = require('npc.include.invop')

local accessoryTypeList = {'戒指', '手镯', '项链'}

-- there is no item price in the item record yet, pay a flat price
-- keep it under what NPChar::getCostItemList() charges to buy, otherwise
-- player can buy from her and sell straight back for free gold
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
                '这里是研究和开发饰品的地方。',
            },
            {
                dialog.link('npc_goto_purchase', '购买', {suffix = '饰品'}),
                dialog.link('npc_goto_trade', '出售', {suffix = '饰品'}),
                dialog.link('npc_goto_repair', '修理', {suffix = '饰品'}),
                dialog.link('npc_goto_special_repair', '特殊修理', {suffix = '饰品'}),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end
    end,

    ["npc_goto_purchase"] = function(uid, value)
        dialog.post(uid, '你想要哪种？戒指还是手镯？',
        dialog.link(SYS_ENTER, '前一步'))
        uidPostSell(uid)
    end,

    ["npc_goto_trade"] = function(uid, value)
        dialog.post(uid, '我的任务之一就是随时配备好饰品，以备不时之需。如果有多余的饰品，请卖给我，我给你个合理的价钱。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartTrade(uid, "npc_goto_query_trade", "npc_goto_commit_trade", accessoryTypeList)
    end,

    ["npc_goto_query_trade"] = function(uid, value)
        invop.postQueryTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", accessoryTypeList, tradeGold)
    end,

    ["npc_goto_commit_trade"] = function(uid, value)
        invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", accessoryTypeList, tradeGold)
    end,

    ["npc_goto_repair"] = function(uid, value)
        dialog.post(uid,
        {
            '无论是旧的还是碎了的饰品我都能把它修好。你想修什么？',
            '普通修理会有概率损失饰品的持久上限。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", accessoryTypeList)
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        dialog.post(uid,
        {
            '特殊修理不会损失持久上限，不过饰品精巧，价钱也就贵得多。',
            '请选择要修理的饰品。',
        },
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", accessoryTypeList)
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", accessoryTypeList)
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", accessoryTypeList)
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", accessoryTypeList)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", accessoryTypeList)
    end,
})
