local dialog = require('include.dialog')

setNPCSell({'破山剑', '旋风流星刀', '破魂'})

local function randomHeadString()
    if math.random(0, 1) == 0 then
        return '欢迎来到大城市比奇省！'
    else
        return '有什么可以为你效劳的吗？'
    end
end

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid, string.format('客官%s你好我是%s，%s<emoji id="0"/>', uidQueryName(uid), getNPCName(), randomHeadString()),
        {
            dialog.link('event_post_sell', '购买武器'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid)
    end,
})
