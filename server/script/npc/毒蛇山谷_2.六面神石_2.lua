setNPCLook(56)
setNPCGLoc(314, 193)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '攻城战地域', map = '沙巴克城_3', x = 223, y = 158, gold = 500},
})
