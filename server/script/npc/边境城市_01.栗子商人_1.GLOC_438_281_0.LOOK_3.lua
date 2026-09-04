-- converted from Envir/Market_Def/10ChestnutMarket_Kugkyung-01.txt
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
            uidPostXML(uid,
            [[
                <layout>
                    <par>跟你这种人我无话可说。</par>
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

        table.insert(par, [[<par><event id="npc_gold1">带来了金色栗子，你要卖吗？</event></par>]])
        table.insert(par, [[<par><event id="npc_silver1">带来了银色栗子，你要卖吗？</event></par>]])
        table.insert(par, [[<par><event id="npc_copper1">带来了铜色栗子，你要卖吗？</event></par>]])
        table.insert(par, [[<par><event id="npc_brown1">带来了褐色栗子，你要卖吗？</event></par>]])
        table.insert(par, string.format([[<par><event id="%s" close="1">结束</event></par>]], SYS_EXIT))

        uidPostXML(uid, string.format('<layout>%s</layout>', table.concat(par)))
    end,

    -- legacy @gold1
    npc_gold1 = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这件事现在还办不了。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    -- legacy @silver1
    npc_silver1 = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这件事现在还办不了。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    -- legacy @copper1
    npc_copper1 = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这件事现在还办不了。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    -- legacy @brown1
    npc_brown1 = function(uid, value)
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
