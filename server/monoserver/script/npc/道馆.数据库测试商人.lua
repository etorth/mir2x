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
setNPCLook(8)
setNPCGLoc(404, 124)

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        sayXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，我可以给你展示系统所有的账号！<emoji id="0"/></par>
                <par></par>
                <par><event id="npc_goto_1">展示</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_NPCDONE))
    end,

    ["npc_goto_1"] = function(uid, value)
        local result = dbQuery('select * from tbl_account')
        local parStr = ''
        for _, row in ipairs(result) do
            parStr = parStr .. string.format('<par>fld_id: %d, fld_account: %s</par>', row.fld_dbid, row.fld_account)
        end

        sayXML(uid,
        [[
            <layout>
                <par>数据库玩家账号有：</par>
                <par></par>
                %s
                <par></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], parStr, SYS_NPCDONE)
    end,
}
