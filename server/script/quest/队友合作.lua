setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, value)
        uidRemoteCall(getNPCharUID('道馆_1', '士官_1'), uid,
        [[
            local playerUID = ...
            local dialog = require('include.dialog')
            dialog.post(playerUID, '和队友开始挑战珐玛大陆的怪物吧！',
            dialog.link(SYS_EXIT, '好的'))
        ]])
        setQuestState{uid=uid, state='quest_setup_player_die_trigger'}
    end,

    quest_setup_player_die_trigger = function(uid, value)
        uidRemoteCall(uid, getUID(), uid,
        [[
            local questUID, playerUID = ...
            addTrigger(SYS_ON_DIE, function()
                uidRemoteCall(questUID, playerUID,
                [=[
                    local playerUID = ...
                    setQuestState{uid=playerUID, state='quest_abort_by_die'}
                ]=])
                return true
            end)
        ]])
        setQuestState{uid=uid, state='quest_setup_player_offline_trigger'}
    end,

    quest_setup_player_offline_trigger = function(uid, value)
        uidRemoteCall(uid, getUID(), uid,
        [[
            local questUID, playerUID = ...
            addTrigger(SYS_ON_OFFLINE, function()
                uidRemoteCall(questUID, playerUID,
                [=[
                    local playerUID = ...
                    setQuestState{uid=playerUID, state='quest_abort_by_offline'}
                ]=])
                return true
            end)
        ]])
        setQuestState{uid=uid, state='quest_setup_kill_trigger'}
    end,

    quest_setup_kill_trigger = function(uid, value)
        local triggerKey = uidRemoteCall(uid, getQuestName(), getThreadAddress(),
        [[
            local questName, threadAddress = ...
            local killCount = 0

            return addTrigger(SYS_ON_KILL, function(monsterID)
                local monsterName = getMonsterName(monsterID)
                if monsterName then
                    killCount = killCount + 1
                    postString([=[任务『%s』正在进行中，消灭一只%s，你已经消灭%d只怪物。]=], questName, monsterName, killCount)

                    if killCount >= 5 then
                        sendNotify(threadAddress, true)
                        return true
                    end
                end
            end)
        ]])

        if waitNotify(30 * 1000) then
            uidRemoteCall(uid, [[ postString([=[挑战成功！]=]) ]])
            setQuestState{uid=uid, state=SYS_DONE}

        else
            uidRemoteCall(uid, triggerKey,
            [[
                local triggerKey = ...
                deleteTrigger(triggerKey)
                postString([=[30秒已过，你超时了，挑战失败。]=])
            ]])
        end
    end,

    quest_abort_by_die = function(uid, args)
        local team = getQuestTeam(uid)
        for _, playerUID in ipairs(team[SYS_QUESTFIELD.TEAM.ROLELIST]) do
            uidRemoteCall(playerUID, uid, getQuestName(),
            [[
                local deadPlayerUID, questName = ...
                if deadPlayerUID == getUID() then
                    postString([=[你死了，任务『%s』终止。]=], questName)
                else
                    postString([=[你的队友『%s』死了，任务『%s』终止。]=], server.player.getName(deadPlayerUID), questName)
                end
            ]])

            if playerUID ~= uid then
                runQuestThread(function()
                    setQuestState{uid=playerUID, state=SYS_DONE}
                end)
            end
        end
        setQuestState{uid=uid, state=SYS_DONE}
    end,

    quest_abort_by_offline = function(uid, args)
        setQuestState{uid=uid, state=SYS_DONE}
    end,
})

uidRemoteCall(getNPCharUID('道馆_1', '士官_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, value)
            local currState = uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return dbGetQuestState(playerUID)
            ]=])

            if currState == SYS_DONE then
                dialog.post(uid, questPath, '你已经完成挑战任务。',
                dialog.link(SYS_EXIT, '退出'))

            elseif currState ~= nil then
                dialog.post(uid, questPath, '请继续你的挑战任务。',
                dialog.link(SYS_EXIT, '退出'))

            else
                local teamLeader = uidRemoteCall(uid, [=[ return getTeamLeader() ]=])
                if not teamLeader then
                    dialog.post(uid, questPath, '请先组建一个队伍',
                    dialog.link(SYS_EXIT, '退出'))

                elseif teamLeader ~= uid then
                    dialog.post(uid, questPath, '你不是队长',
                    dialog.link(SYS_EXIT, '退出'))

                else
                    local teamMemberList = uidRemoteCall(uid, [=[ return getTeamMemberList() ]=])
                    if #teamMemberList >= 2 then
                        dialog.post(uid, questPath, '你拥有一个队伍，愿意接受任务吗？',
                        {
                            dialog.link('npc_accept_quest', '同意'),
                            dialog.link(SYS_EXIT, '退出'),
                        })

                    elseif uidRemoteCall(uid, [=[ return getLevel() ]=]) < 7 then
                        dialog.post(uid, questPath, '你的等级太低，请至少添加一名队友结队冒险，或者升到7级再来找我。',
                        dialog.link(SYS_EXIT, '退出'))

                    else
                        dialog.post(uid, questPath, '你确定独自冒险吗？',
                        {
                            dialog.link('npc_accept_quest', '同意'),
                            dialog.link(SYS_EXIT, '退出'),
                        })
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
