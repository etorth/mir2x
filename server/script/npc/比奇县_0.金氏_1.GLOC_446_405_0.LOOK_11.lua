local dialog = require('include.dialog')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid, '你是来卖肉的？',
        {
            dialog.link('npc_goto_sell', '卖', {suffix = '肉'}),
            dialog.link(SYS_EXIT, '结束'),
        })
    end,

    ["npc_goto_sell"] = function(uid, value)
        dialog.post(uid,
        {
            '高价收购优质肉。',
            '沾上土的或被火烧过的肉廉价收购。',
        },
        {
            dialog.link(SYS_ENTER, '前一步'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    end,
})
