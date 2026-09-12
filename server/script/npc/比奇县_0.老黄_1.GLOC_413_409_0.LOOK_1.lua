local dialog = require('include.dialog')
-- converted from Envir/Market_Def/04PotionMake_Bichon2-0.txt
--
-- an item crafter, legacy Market_Def/04PotionMake_*.txt
--
-- 请求制作 delegates to QuestDiary/Make_Item/Menu.txt, the crafting system, which is not
-- converted. this NPC sells nothing and repairs nothing, so it is not a merchant and is
-- on no npc/include/merchant template

local greet =
{
    '你知道组合许多不同的材料制作物品的乐趣吗？哈哈哈！',
    '你收集材料过来的话我给你制作吧。我想让大家知道我的喜悦。。哈哈',
}

setEventHandler
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            dialog.post(uid, '不良的朋友啊。。马上在我面前消失。。',
            dialog.link(SYS_EXIT, '结束'))
            return
        end

        local par = {}
        for _, line in ipairs(greet) do
            table.insert(par, line)
        end

        dialog.post(uid, par,
        {
            dialog.link('npc_makeitem', '请求制作'),
            dialog.link('npc_helpmakeitem', '打听关于制作的事情'),
            dialog.link(SYS_EXIT, '结束'),
        })
    end,

    -- legacy @Makeitem
    npc_makeitem = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,

    -- legacy @HelpMakeitem
    npc_helpmakeitem = function(uid, value)
        dialog.post(uid, '这件事现在还办不了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,
}
