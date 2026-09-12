local dialog = require('include.dialog')
-- converted from Envir/Market_Def/10ChestnutMarket_Eunhang-02.txt
--
-- the coloured-chestnut exchange, legacy Market_Def/10ChestnutMarket_*.txt
--
-- each 带来了X栗子 option delegates to QuestDiary/Event/Chestnut/Event3.txt, a seasonal
-- event that is not converted. this NPC is not a merchant: no [Goods], no 购买 and no
-- 出售, only barter, which is why it is on no npc/include/merchant template

local greet =
{
    '欢迎光临，对，我就是买栗子的。 如果你能给我找来那些味道又好，营养又好的栗子，我就送你一份大礼。',
    '你有栗子吗？',
}

setEventHandler
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            dialog.post(uid, '跟你这种人我无话可说。',
            dialog.link(SYS_EXIT, '结束'))
            return
        end

        local par = {}
        for _, line in ipairs(greet) do
            table.insert(par, line)
        end

        dialog.post(uid, par,
        {
            dialog.link('npc_gold1', '带来了金色栗子，你要卖吗？'),
            dialog.link('npc_silver1', '带来了银色栗子，你要卖吗？'),
            dialog.link('npc_copper1', '带来了铜色栗子，你要卖吗？'),
            dialog.link('npc_brown1', '带来了褐色栗子，你要卖吗？'),
            dialog.link(SYS_EXIT, '结束'),
        })
    end,

    -- legacy @gold1
    npc_gold1 = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    -- legacy @silver1
    npc_silver1 = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    -- legacy @copper1
    npc_copper1 = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    -- legacy @brown1
    npc_brown1 = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,
}
