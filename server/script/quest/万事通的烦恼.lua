function main()
    uidExecute(getNPCharUID('道馆_1', '万事通_1'),
    [[
        return setQuestHandler('%s', {
            ['%s'] = function(uid, value)
                uidPostXML(uid, {'%s', '%s'}, [=[
                    <layout>
                        <par>这是一个任务系统的测试，这个脚本可以覆盖默认对话脚本。</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=])
            end
        })
    ]], getQuestName(), SYS_NPCINIT, SYS_EPQST, getQuestName(), SYS_NPCDONE)
end
