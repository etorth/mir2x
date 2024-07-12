setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, value)
        uidRemoteCall(getNPCharUID('道馆_1', '士官_1'), uid,
        [[
            local playerUID = ...
            uidPostXML(playerUID,
            [=[
                <layout>
                    <par>和队友开始挑战珐玛大陆的怪物吧！</par>
                    <par></par>
                    <par><event id="%s">好的</event></par>
                </layout>
            ]=], SYS_EXIT)
        ]])
        setQuestState{uid=uid, state='quest_setup_kill_trigger'}
    end,

    quest_setup_kill_trigger = function(uid, value)
        local triggerKey = uidRemoteCall(uid, getThreadAddress(),
        [[
            local threadAddress = ...
            local killCount = 0

            return addTrigger(SYS_ON_KILL, function(monsterID)
                local monsterName = getMonsterName(monsterID)
                if monsterName then
                    killCount = killCount + 1
                    postString([=[挑战正在进行中，消灭一只%s，你已经消灭%d只怪物。]=], monsterName, killCount)

                    if killCount >= 5 then
                        sendNotify(threadAddress, true)
                        return true
                    end
                end
            end)
        ]])

        if waitNotify(10 * 1000) then
            uidRemoteCall(uid, [[ postString([=[挑战成功！]=]) ]])
            setQuestState{uid=uid, state=SYS_DONE}
        else
            uidRemoteCall(uid, triggerKey,
            [[
                local triggerKey = ...
                deleteTrigger(triggerKey)
                postString([=[挑战失败，你超时了。]=])
            ]])
        end
    end,
})

uidRemoteCall(getNPCharUID('道馆_1', '士官_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, value)
            local currState = uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return dbGetQuestState(playerUID)
            ]=])

            if currState == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你已经完成挑战任务。</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)

            elseif currState ~= nil then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>请继续你的挑战任务。</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)

            else
                local teamLeader = uidRemoteCall(uid, [=[ return getTeamLeader() ]=])
                if not teamLeader then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请先组建一个队伍</par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                elseif teamLeader ~= uid then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你不是队长</par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                else
                    local teamMemberList = uidRemoteCall(uid, [=[ return getTeamMemberList() ]=])
                    if #teamMemberList >= 2 then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你拥有一个队伍，愿意接受任务吗？</par>
                                <par><event id="npc_accept_quest">同意</event></par>
                                <par><event id="%s"              >退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    elseif uidRemoteCall(uid, [=[ return getLevel() ]=]) < 7 then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你的等级太低，请至少添加一名队友结队冒险，或者升到7级再来找我。</par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你确定独自冒险吗？</par>
                                <par><event id="npc_accept_quest">同意</event></par>
                                <par><event id="%s"              >退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end
            end
        end,

        npc_accept_quest = function(uid, value)
            local teamRoleList = uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                setQuestTeam{uid=playerUID, randRole=true, propagate=true}
                return getQuestTeam(playerUID)[SYS_QUESTFIELD.TEAM.ROLELIST]
            ]=])

            for _, teamRole in ipairs(teamRoleList) do
                uidRemoteCall(questUID, teamRole,
                [=[
                    local teamRole = ...
                    setQuestState{uid=teamRole, state=SYS_ENTER}
                ]=])
            end
        end,
    })
]])
