function main()
    uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
    [[
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                if uidExecute(uid, [=[ return getQuestState('初出江湖') ]=]) ~= SYS_EXIT then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我不和无名之辈来往。</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>妈耶！你来啦！</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end,
        })
    ]], getQuestName())
end
