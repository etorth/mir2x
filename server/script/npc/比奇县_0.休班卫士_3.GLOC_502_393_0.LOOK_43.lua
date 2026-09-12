local dialog = require('include.dialog')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid,
        {
            '我是这比奇省里资历最深的卫士。',
            '嗯...虽说什么，那也不是特别的...',
        },
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
