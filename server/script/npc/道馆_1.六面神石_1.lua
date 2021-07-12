setNPCLook(56)
setNPCGLoc(416, 179)

local tp = require('npc.include.teleport')
tp.setTeleport('<par>你想去哪里？</par>',
{
    {name = '比奇城', map = '比奇县_0', x = 370, y = 335, gold = 500},
    {},

    {name = '毒蛇山村', map = '毒蛇山谷_2', x = 306, y = 245, gold = 1000},
    {name = '银杏山村', map = '银杏山谷_02', x = 249, y = 145, gold = 1000},
    {name = '边境村庄', map = '边境城市_01', x = 456, y = 217, gold = 1000},
    {},

    {name = '潘夜岛村', map = '潘夜岛_8', x = 288, y = 242, gold = 1500},
    {name = '盟重土城', map = '盟重县_74', x = 349, y = 330, gold = 1500},
    {},

    {map = '沙漠土城_5', x = 204, y = 290, gold = 2000},
    {},

    {map = '绿洲_4', x = 435, y = 84, gold = 2500},
    {},

    {name = '攻城战地域', map = '沙巴克城_3', x = 222, y = 160, gold = 500, level = 7},
    {},
})
