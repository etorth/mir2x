-- =====================================================================================
--
--       Filename: 道馆.物品展示商人.lua
--        Created: 11/22/2020 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: lua
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

-- NPC script
-- provides the table: processNPCEvent for event processing

addLog(LOGTYPE_INFO, 'NPC %s sources %s', getNPCFullName(), getFileName())
setNPCLook(5)
setNPCGLoc(402, 122)

-- list all possible items for debug
-- setNPCSell() requires a variadic string list, not a table
-- pass a table back to C++ requires to add a global varialbe and pass back the variable name
do
    local itemID = 1
    local itemNameList = {}

    while(true) do
        local itemName = getItemName(itemID)
        if itemName == '' then
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
        sayXML(uid, string.format(
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
