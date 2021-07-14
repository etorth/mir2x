setNPCLook(57)
setNPCGLoc(28, 31)

local tp = require('npc.include.teleport')
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        tp.uidReqSpaceMove(uid, '潘夜神殿5层_D1105', 221, 102)
    end,
}
