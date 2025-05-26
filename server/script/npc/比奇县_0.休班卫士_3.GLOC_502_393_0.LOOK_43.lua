setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>我是这比奇省里资历最深的卫士。</par>
                <par>嗯...虽说什么，那也不是特别的...</par>
                <par></par>
                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
