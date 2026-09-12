local dialog = require('include.dialog')

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

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid, string.format('客官%s你好我是%s，我这里有所有的物品哦！<emoji id="0"/>', uidQueryName(uid), getNPCName()),
        {
            dialog.link('event_post_sell', '购买'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid)
    end,
})
