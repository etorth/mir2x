-- =====================================================================================
--
--       Filename: 道馆.铁匠.lua
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

addLog(LOGTYPE_INFO, string.format('NPC %s sources %s', getNPCFullName(), getFileName()))
local function randomHeadString()
    if math.random(0, 1) == 0 then
        return '与衣服鞋子有关的都可以找我！'
    else
        return '有什么可以为你效劳的吗？'
    end
end

local function getSell()
    -- return {'太阳水', '紫金靴', '乌金靴', '青铜头盔', '虎面头盔', '破山剑', '旋风流星刀', '破魂'}
    --
    local result = {}
    local itemList = getSellItemList();

    for i = 1, #itemList do
        table.insert(result, itemList[i])
    end
    return result
end

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        sayXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，%s<emoji id="0"/></par>
                <par></par>
                <par><event id="event_post_sell">购买</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), randomHeadString(), SYS_NPCDONE))
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid, getSell())
    end,
}
