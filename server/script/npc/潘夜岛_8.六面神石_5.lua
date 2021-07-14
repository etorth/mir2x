setNPCLook(57)
setNPCGLoc(424, 239)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '边境村庄', map = '边境城市_01', x = 464, y = 355, gold = 500},
    {name = '潘夜石窟较近处地下城', map = '潘夜岛_8', x = 449, y = 578, gold = 500},
    {name = '潘夜神殿较近处地下城', map = '潘夜岛_8', x = 669, y = 387, gold = 500},
    {name = '比奇城', map = '比奇县_0', x = 371, y = 335, gold = 500},
    {},

    {name = '银杏山村', map = '银杏山谷_02', x = 250, y = 143, gold = 1000},
    {name = '道馆村', map = '道馆_1', x = 417, y = 178, gold = 1000},
    {},

    {name = '盟重土城', map = '盟重县_74', x = 350, y = 328, gold = 1500},
    {},

    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 307, y = 243, gold = 2000},
    {map = '沙漠土城_5', x = 205, y = 288, gold = 2000},
    {},

    {map = '绿洲_4', x = 436, y = 82, gold = 2500},
    {map = '流放岛_81', x = 130, y = 264, gold = 10000},
})
