function main()
    uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
    [[
        return setQuestHandler('%s', {
            ['%s'] = function(uid, value)
                uidPostXML(uid, {'%s', '%s'}, [=[
                    <layout>
                        <par>这是一个测试</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=])
            end
        })
    ]], getQuestName(), SYS_NPCINIT, SYS_EPQST, getQuestName(), SYS_NPCDONE)
end
