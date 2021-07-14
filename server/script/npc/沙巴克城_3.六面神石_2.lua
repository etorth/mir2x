setNPCLook(56)
setNPCGLoc(294, 539)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '道馆村', map = '道馆_1', x = 417, y = 178, gold = 500},
    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 315, y = 192, gold = 500},
})
