local tp = require('npc.include.teleport')
setEventHandler(
{
    [SYS_NPCINIT] = function(uid, value)
        tp.uidReqSpaceMove(uid, '潘夜神殿大厅_D1110', 29, 30)
    end,
})
