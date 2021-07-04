setNPCLook(3)
setNPCGLoc(382, 141)

local function onSellChestnut(uid, chestnutName, currTagName, lastTagName)
    local priceTable = {
        ['金色栗子'] = 10000,
        ['银色栗子'] =  5000,
        ['铜色栗子'] =  2000,
        ['褐色栗子'] =  1000,
    }

    if priceTable[chestnutName] == nil then
        fatalPrintf('Invalid chestnut name: %s', tostring(chestnutName))
    end

    if uidUseItem(uid, chestnutName, 1) then
        uidPostGold(uid, priceTable[chestnutName])
        uidPostXML(uid,
        [[
            <layout>
                <par>请收下<t color="RED">%d</t>金币</par>
                <par></par>

                <par><event id="%s">还卖其它的%s</event></par>
                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], priceTable[chestnutName], currTagName, chestnutName, lastTagName, SYS_NPCDONE)
    else
        uidPostXML(uid,
        [[
            <layout>
                <par>哼，你没带%s来，快去找！</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], chestnutName, SYS_NPCDONE)
    end
end

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>跟你这种人我无话可说。</par>
                    <par></par>

                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>欢迎光临，对，我就是买栗子的。 如果你能给我找来那些味道又好，营养又好的栗子，我就送你一份大礼。</par>
                    <par>你有栗子吗？</par>
                    <par></par>

                    <par><event id="npc_goto_1">带来了金色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_2">带来了银色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_3">带来了铜色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_4">带来了褐色栗子，你要卖吗？</event></par>
                    <par><event id="%s">马上去给你找</event></par>
                </layout>
            ]], SYS_NPCDONE)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        onSellChestnut(uid, '金色栗子', "npc_goto_1", SYS_NPCINIT)
    end,

    ["npc_goto_2"] = function(uid, value)
        onSellChestnut(uid, '银色栗子', "npc_goto_2", SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        onSellChestnut(uid, '铜色栗子', "npc_goto_3", SYS_NPCINIT)
    end,

    ["npc_goto_4"] = function(uid, value)
        onSellChestnut(uid, '褐色栗子', "npc_goto_4", SYS_NPCINIT)
    end,
}
