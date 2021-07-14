setNPCLook(56)
setNPCGLoc(411, 287)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '盟重土城', map = '盟重县_74', x = 350, y = 328, gold = 500},
    {},

    {map = '沙漠土城_5', x = 205, y = 288, gold = 1000},
    {},

    {name = '绿洲村', map = '绿洲_4', x = 436, y = 82, gold = 1500},
})
