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

    if uidRemove(uid, chestnutName, 1) then
        uidGrantGold(uid, priceTable[chestnutName])
        uidPostXML(uid,
        [[
            <layout>
                <par>请收下<t color="red">%d</t>金币</par>
                <par></par>

                <par><event id="%s" arg="%s">还卖其它的%s</event></par>
                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], priceTable[chestnutName], currTagName, chestnutName, chestnutName, lastTagName, SYS_NPCDONE)
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

                    <par><event id="npc_goto_trade" arg="金色栗子">带来了金色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_trade" arg="银色栗子">带来了银色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_trade" arg="铜色栗子">带来了铜色栗子，你要卖吗？</event></par>
                    <par><event id="npc_goto_trade" arg="褐色栗子">带来了褐色栗子，你要卖吗？</event></par>
                    <par><event id="%s">马上去给你找</event></par>
                </layout>
            ]], SYS_NPCDONE)
        end
    end,

    ["npc_goto_trade"] = function(uid, value)
        onSellChestnut(uid, value, "npc_goto_trade", SYS_NPCINIT)
    end,
}
