setNPCLook(56)
setNPCGLoc(456, 216)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '比奇城', map = '比奇县_0', x = 380, y = 443, gold = 500},
    {},

    {name = '道馆村', map = '道馆_1', x = 417, y = 178, gold = 1000},
    {name = '银杏山村', map = '银杏山谷_02', x = 250, y = 143, gold = 1000},
    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 307, y = 243, gold = 1000},
})
