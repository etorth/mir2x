local dialog = require('include.dialog')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        -- set 0 : 0 ~ 129
        -- set 1 : 0 ~  69

        emojis = {}
        for i = 0, 169 do
            table.insert(emojis, string.format('<emoji id="%d"/>', i))
        end

        for i = 0, 69 do
            table.insert(emojis, string.format('<emoji set="1" id="%d"/>', i))
        end

        dialog.post(uid,
        {
            string.format('客官%s你好我是%s，我展示所有表情包子！<emoji id="0"/>', uidQueryName(uid), getNPCName()),
            table .concat(emojis, ' '),
        },
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
