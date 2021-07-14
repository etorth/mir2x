setNPCLook(57)
setNPCGLoc(129, 265)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>我可以免费送你去潘夜岛村，你想去吗？</par>',
{
    {name = '潘夜岛村', map = '潘夜岛_8', x = 289, y = 240},
})
