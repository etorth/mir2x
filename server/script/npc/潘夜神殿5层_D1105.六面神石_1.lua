setNPCLook(57)
setNPCGLoc(219, 99)

local tp = require('npc.include.teleport')
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        tp.uidReqSpaceMove(uid, '潘夜神殿大厅_D1110', 30, 33)
    end,
}
