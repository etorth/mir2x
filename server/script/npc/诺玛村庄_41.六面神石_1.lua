setNPCLook(56)
setNPCGLoc(184, 136)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>我可以送你去绿洲村，你要去吗？</par>',
{
    {name = '绿洲村', map = '绿洲_4', x = 436, y = 82, gold = 5000},
})
