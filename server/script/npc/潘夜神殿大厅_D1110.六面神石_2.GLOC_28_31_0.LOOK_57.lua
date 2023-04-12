local tp = require('npc.include.teleport')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        tp.uidReqSpaceMove(uid, '潘夜神殿5层_D1105', 220, 98)
    end,
})
