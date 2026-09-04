-- converted from Envir/Market_Def/04PotionMake_Samak-5.txt
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
            uidPostXML(uid,
            [[
                <layout>
                    <par>不良的朋友啊。。马上在我面前消失。。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]], SYS_EXIT)
            return
        end

        local par = {}
        for _, line in ipairs(greet) do
            table.insert(par, string.format('<par>%s</par>', line))
        end

        table.insert(par, [[<par><event id="npc_makeitem">请求制作</event></par>]])
        table.insert(par, [[<par><event id="npc_helpmakeitem">打听关于制作的事情</event></par>]])
        table.insert(par, string.format([[<par><event id="%s" close="1">结束</event></par>]], SYS_EXIT))

        uidPostXML(uid, string.format('<layout>%s</layout>', table.concat(par)))
    end,

    -- legacy @Makeitem
    npc_makeitem = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这件事现在还办不了。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    -- legacy @HelpMakeitem
    npc_helpmakeitem = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这件事现在还办不了。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,
}
