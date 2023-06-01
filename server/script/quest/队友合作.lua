function main()
    uidExecute(getNPCharUID('道馆_1', '士官_1'),
    [[
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                local teamLeader = uidExecute(uid, [=[ return getTeamLeader() ]=])
                if not teamLeader then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请先组建一个队伍</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                elseif teamLeader ~= uid then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你不是队长</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你拥有一个队伍</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end,
        })
    ]], getQuestName())
end
