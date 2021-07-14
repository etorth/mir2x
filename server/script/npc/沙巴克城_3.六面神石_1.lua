setNPCLook(56)
setNPCGLoc(222, 159)

-- 沙巴克城 has 4 teleports
-- all these 4 teleports uses same destination table

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '道馆村', map = '道馆_1', x = 417, y = 178, gold = 500},
    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 315, y = 192, gold = 500},
})
