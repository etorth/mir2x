do
    local itemID = 1
    local itemNameList = {}

    while(true) do
        local itemName = getItemName(itemID)
        if not hasChar(itemName) then
            break
        end

        if itemName ~= '未知' then
            table.insert(itemNameList, itemName)
        end

        itemID = itemID + 1
    end

    setNPCSell(itemNameList)
end

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，我这里有所有的物品哦！<emoji id="0"/></par>
                <par></par>
                <par><event id="event_post_sell">购买</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_NPCDONE))
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid)
    end,
}
