local tp = require('npc.include.teleport')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        tp.uidReqSpaceMove(uid, '潘夜神殿大厅_D1110', 29, 30)
    end,
})
