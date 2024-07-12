_G.minQuestLevel = 7
_G.questDoneFlag = 'done_quest_万事通的烦恼'

addQuestTrigger(SYS_ON_LEVELUP, function(playerUID, oldLevel, newLevel)
    if oldLevel < minQuestLevel and newLevel >= minQuestLevel then
        uidRemoteCall(playerUID, newLevel,
        [[
            local newLevel = ...
            postString([=[恭喜你升到%d级，快去找道馆的万拍子看看，他好像正需要人帮忙。]=], newLevel)
        ]])
    end
end)

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, value)
        uidRemoteCall(getNPCharUID('道馆_1', '万事通_1'), uid,
        [[
            local playerUID = ...
            uidPostXML(playerUID,
            [=[
                <layout>
                    <par>多谢少侠出手相助！最近城外钉耙猫肆虐，请少侠出城斩杀一只<t color="red">钉耙猫</t>，事后我万拍子有礼物奉上！</par>
                    <par></par>
                    <par><event id="%s">好的</event></par>
                </layout>
            ]=], SYS_EXIT)
        ]])

        setQuestState{uid=uid, state='quest_setup_kill_trigger'}
    end,

    ['quest_setup_kill_trigger'] = function(uid, value)
        uidRemoteCall(uid, getUID(),
        [[
            local questUID = ...
            addTrigger(SYS_ON_KILL, function(monsterID)
                if getMonsterName(monsterID) == '钉耙猫' then
                    postString([=[已经消灭一只钉耙猫，去找万拍子交谈获取礼物。]=])
                    uidRemoteCall(questUID, getUID(),
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, state='quest_killed_monster'}
                    ]=])
                    return true
                end
            end)
        ]])

        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于钉耙猫',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那<t color="red">钉耙猫</t>原本是温顺的小猫，近年不知为何开始变得像猩猩一般强健，还偷走附近农户的钉耙向行人胡乱攻击。</par>
                            <par>少侠你可要千万当心！</par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    ['quest_killed_monster'] = function(uid, value)
        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领取奖励',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>少侠神勇！我万拍子果然没有看错，这是我的一点心意，还望少侠收下！</par>
                            <par><event id="%s">关闭</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(uid,
                    [=[
                        addItem(getItemID('铁剑'), 1)
                        addItem(getItemID('太阳水'), 5)
                    ]=])

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, state=SYS_DONE}
                    ]=])
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('道馆_1', '万事通_1'), getUID(), getQuestName(), minQuestLevel, questDoneFlag,
[[
    local questUID, questName, minQuestLevel, questDoneFlag = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            local level, hasDoneFlag = uidRemoteCall(uid, questDoneFlag,
            [=[
                local questDoneFlag = ...
                return getLevel(), dbHasFlag(questDoneFlag)
            ]=])

            if level < minQuestLevel or hasDoneFlag then
                return false
            end

            local questState = uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return dbGetQuestState(playerUID)
            ]=])

            if questState ~= nil then
                return false
            end

            return true
        end,

        [SYS_ENTER] = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>我万拍子近日愁眉苦脸，都是被那城门外的<t color="red">钉耙猫</t>害的啊！少侠你已经<t color="green">%d</t>级了，愿意帮助老夫吗？</par>
                    <par><event id="npc_accept_quest">同意</event></par>
                    <par><event id="%s"              >退出</event></par>
                </layout>
            ]=], uidRemoteCall(uid, [=[ return getLevel() ]=]), SYS_EXIT)
        end,

        ['npc_accept_quest'] = function(uid, value)
            uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                setQuestState{uid=playerUID, state=SYS_ENTER}
            ]=])

            uidRemoteCall(uid, questDoneFlag,
            [=[
                local questDoneFlag = ...
                dbAddFlag(questDoneFlag)
            ]=])
        end,
    })
]])
