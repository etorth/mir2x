local dialog = require('include.dialog')
setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if math.random(0, 1) == 0 then
            dialog.post(uid,
            {
                '呵！今天好想喝酒啊！',
                '可是你兜里空空的，怎么让你给我买酒呢？',
            },
            dialog.link(SYS_EXIT, '关闭'))
        else
            dialog.post(uid,
            {
                '是您哪！',
                '又想要给我点什么了吧！唔，那就给我买一瓶酒吧？！',
                '不！尽管这样也要有卫士的尊严啊！',
            },
            dialog.link(SYS_EXIT, '关闭'))
        end
    end,
})
