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

local tp = require('npc.include.teleport')
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
        tp.uidReqSpaceMove(uid, '比奇省', 100, 100, 10)
    end,

    ["goto_2"] = function(uid, value)
        tp.uidReqSpaceMove(uid, '银杏山谷', 100, 100, 10, 2)
    end,

    ["goto_3"] = function(uid, value)
        tp.uidReqSpaceMove(uid, '沙巴克', 100, 100, 10, 4)
    end,
}
