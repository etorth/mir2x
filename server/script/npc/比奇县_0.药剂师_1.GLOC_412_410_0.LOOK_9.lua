local dialog = require('include.dialog')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid,
        {
            '我现在不再制作药了，制作要的话去找隔壁的老黄吧。',
            '你找我有什么事情吗？',
        },
        dialog.link(SYS_EXIT, '结束'))
    end,
})
