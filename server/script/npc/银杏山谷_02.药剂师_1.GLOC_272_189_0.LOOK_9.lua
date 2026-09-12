local dialog = require('include.dialog')
-- converted from Envir/Market_Def/04PotionMake_Euhang-02.txt
--
-- an item crafter, legacy Market_Def/04PotionMake_*.txt
--
-- 请求制作 delegates to QuestDiary/Make_Item/Menu.txt, the crafting system, which is not
-- converted. this NPC sells nothing and repairs nothing, so it is not a merchant and is
-- on no npc/include/merchant template

local greet =
{
    '很不好意思，我现在不制作药了。',
    '想制作药的话去找比奇县的老黄和 沙漠土城的老郑吧。',
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
            dialog.link('npc_tquest', '对今日的任务进行了解'),
            dialog.link(SYS_EXIT, '结束'),
        })
    end,

    -- legacy @TQuest
    npc_tquest = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_ENTER, '前一步'))
    end,
}
