function main()
    uidExecute(getNPCharUID('道馆_1', '士官_1'),
    [[
        local questUID  = %d
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                local currState = uidExecute(questUID, [=[ return dbGetUIDQuestState(%%d) ]=], uid)
                if currState == SYS_EXIT then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你已经完成挑战任务。</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                elseif currState ~= nil then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请继续你的挑战任务。</par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                else
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
                        local teamMemberList = uidExecute(uid, [=[ return getTeamMemberList() ]=])
                        if #teamMemberList >= 2 then
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>你拥有一个队伍，愿意接受任务吗？</par>
                                    <par><event id="npc_accept_quest">同意</event></par>
                                    <par><event id="%%s"             >退出</event></par>
                                </layout>
                            ]=], SYS_EXIT)

                        elseif uidExecute(uid, [=[ return getLevel() ]=]) < 7 then
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>你的等级太低，请至少添加一名队友结队冒险。</par>
                                    <par><event id="%%s">退出</event></par>
                                </layout>
                            ]=], SYS_EXIT)

                        else
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>你确定独自冒险吗？</par>
                                    <par><event id="npc_accept_quest">同意</event></par>
                                    <par><event id="%%s"             >退出</event></par>
                                </layout>
                            ]=], SYS_EXIT)
                        end
                    end
                end
            end,

            npc_accept_quest = function(uid, value)
                local teamRoleList = uidExecute(questUID,
                [=[
                    setUIDQuestTeam{uid=%%d, randRole=true, propagate=true}
                    return getUIDQuestTeam(%%d)[SYS_QUESTFIELD.TEAM.ROLELIST]
                ]=], uid, uid)

                for _, teamRole in ipairs(teamRoleList) do
                    uidExecute(questUID,
                    [=[
                        setUIDQuestState(%%d, SYS_ENTER)
                    ]=], teamRole)
                end
            end,
        })
    ]], getUID(), getQuestName())

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            uidExecute(getNPCharUID('道馆_1', '士官_1'),
            [[
                uidPostXML(%d,
                [=[
                    <layout>
                        <par>和队友开始挑战珐玛大陆的怪物吧！</par>
                        <par></par>
                        <par><event id="%%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)
            ]], uid)
            setUIDQuestState(uid, 'quest_setup_kill_trigger')
        end,

        quest_setup_kill_trigger = function(uid, value)
            local triggerKey = uidExecute(uid,
            [[
                local killCount = 0
                return addTrigger(SYS_ON_KILL, function(monsterID)
                    if getMonsterName(monsterID) then
                        killCount = killCount + 1
                        postString([=[挑战正在进行中，消灭一只%%s，你已经消灭%%d只怪物。]=], getMonsterName(monsterID), killCount)

                        if killCount >= 5 then
                            sendNotify(%s, true)
                            return true
                        end
                    end
                end)
            ]], asInitString(getThreadAddress()))

            if waitNotify(10 * 1000) then
                uidExecute(uid, [[ postString([=[挑战成功！]=]) ]])
                setUIDQuestState(uid, SYS_DONE)
            else
                uidExecute(uid,
                [[
                    postString([=[挑战失败，你超时了。]=])
                    deleteTrigger(%s)
                ]], asInitString(triggerKey))
            end
        end,
    })
end
