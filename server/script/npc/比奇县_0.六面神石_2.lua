setNPCLook(56)
setNPCGLoc(507, 313)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>我可以送你去毒蛇山村，你要去吗？</par>',
{
    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 307, y = 243, gold = 500},
})
