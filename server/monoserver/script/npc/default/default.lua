-- =====================================================================================
--
--       Filename: default.lua
--        Created: 08/31/2015 08:52:57 PM
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
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        sayXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好，我是%s，欢迎来到传奇旧时光！<emoji id="0"/></par>
                <par>有什么可以为你效劳的吗？</par>
                <par></par>
                <par><event id="event_1">如何玩得开心</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], getUIDString(uid), getNPCName(), SYS_NPCDONE))
    end,

    ["event_1"] = function(uid, value)
        local gold  = queryGold(uid)
        local level = queryLevel(uid)
        if gold < 1000 then
            sayXML(uid, string.format(
            [[
                <layout>
                    <par>穷鬼，先去赚点钱吧！<emoji id="2"/></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE))
        elseif level < 10 then
            sayXML(uid, string.format(
            [[
                <layout>
                    <par>客官你才%d级，先去打怪升级吧！<emoji id="3"/></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], level, SYS_NPCDONE))
        else
            sayXML(uid, string.format(
            [[
                <layout>
                    <par>多多上线打怪升级！<emoji id="1"/></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE))
        end
    end,
}
