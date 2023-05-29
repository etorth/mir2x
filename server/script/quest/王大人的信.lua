function main()
    uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
    [[
        return setQuestHandler('%s', {
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, {SYS_EPQST, '%s'}, [=[
                    <layout>
                        <par>这是一个测试</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=])
            end
        })
    ]], getQuestName(), getQuestName(), SYS_EXIT)
end
