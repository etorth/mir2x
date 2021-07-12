setNPCLook(56)
setNPCGLoc(498, 463)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>我可以送你去银杏山村，你要去吗？</par>',
{
    {name = '银杏山村', map = '银杏山谷_02', x = 250, y = 143, gold = 500},
})
