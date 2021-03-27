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
setNPCLook(56)
setNPCGLoc(416, 179)

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        sayXML(uid,
        [[
            <layout>
                <par>客官%s你好，我是%s，欢迎来到传奇旧时光！<emoji id="0"/></par>
                <par>你想去哪里呢？</par>
                <par></par>
                <par><event id="goto_1">比奇省</event></par>
                <par><event id="goto_2">银杏山谷</event></par>
                <par><event id="goto_3">沙巴克</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_NPCDONE)
    end,

    ["goto_1"] = function(uid, value)
        local gold  = uidQueryGold(uid)
        local level = uidQueryLevel(uid)
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

    ["goto_2"] = function(uid, value)
        local gold  = uidQueryGold(uid)
        local level = uidQueryLevel(uid)
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

    ["goto_3"] = function(uid, value)
        local gold  = uidQueryGold(uid)
        local level = uidQueryLevel(uid)
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
