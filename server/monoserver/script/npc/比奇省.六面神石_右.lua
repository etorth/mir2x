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
setNPCGLoc(507, 313)

local tp = require('npc.include.teleport')
tp.setTeleport({
    {map = '比奇省',   x = 447, y = 386, gold = 10},
    {map = '银杏山谷', x = 246, y = 200, gold = 25, level = 2},
    {map = '沙巴克',   x = 216, y = 148, gold = 50, level = 3},
})
