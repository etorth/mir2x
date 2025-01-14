setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>我现在不再制作药了，制作要的话去找隔壁的老黄吧。</par>
                <par>你找我有什么事情吗？</par>
                <par></par>
                <par><event id="%s" close="1">结束</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
