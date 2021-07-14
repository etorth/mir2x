setNPCLook(56)
setNPCGLoc(435, 83)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {map = '沙漠土城_5', x = 113, y = 176, gold = 500},
    {name = '蚂蚁洞穴入口', map = '沙漠_6', x = 274, y = 730, gold = 500},
    {},

    {name = '盟重土城', map = '盟重县_74', x = 272, y = 266, gold = 1000},
    {name = '边境村庄', map = '边境城市_01', x = 412, y = 286, gold = 1500},
    {name = '比奇城', map = '比奇县_0', x = 380, y = 443, gold = 2000},
    {name = '潘夜岛村', map = '潘夜岛_8', x = 289, y = 240, gold = 2000},
    {},

    {name = '道馆村', map = '道馆_1', x = 417, y = 178, gold = 2500},
    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 307, y = 243, gold = 2500},
    {name = '银杏山村', map = '银杏山谷_02', x = 250, y = 143, gold = 2500},
    {},

    {name = '诺玛村庄', map = '诺玛村庄_41', x = 185, y = 135, gold = 5000},
    {},
})
