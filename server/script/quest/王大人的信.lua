function main()
    uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
    [[
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>这是一个测试</par>
                        <par><event id="%%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end
        })
    ]], getQuestName())
end
