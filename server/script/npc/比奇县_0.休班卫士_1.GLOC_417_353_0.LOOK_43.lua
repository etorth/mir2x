setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if math.random(0, 1) == 0 then
            uidPostXML(uid,
            [[
                <layout>
                    <par>呵！今天好想喝酒啊！</par>
                    <par>可是你兜里空空的，怎么让你给我买酒呢？</par>
                    <par></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_EXIT)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>是您哪！</par>
                    <par>又想要给我点什么了吧！唔，那就给我买一瓶酒吧？！</par>
                    <par>不！尽管这样也要有卫士的尊严啊！</par>
                    <par></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_EXIT)
        end
    end,
})
