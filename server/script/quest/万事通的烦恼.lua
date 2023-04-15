function main()
    uidExecute(getNPCharUID('道馆_1', '万事通_1'),
    [[
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName, {
            [SYS_CHECKACTIVE] = function(uid)
                return uidExecute(uid, [=[ return getLevel() ]=]) >= 7
            end,

            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath, [=[
                    <layout>
                        <par>我万拍子近日愁眉苦脸，都是被那城门外的钉耙猫害的啊！少侠你已经%%d级了，愿意帮助老夫吗？</par>
                        <par><event id="npc_goto_1">同意</event></par>
                        <par><event id="%%s"       >退出</event></par>
                    </layout>
                ]=], uidExecute(uid, [=[ return getLevel() ]=]), SYS_EXIT)
            end,

            ['npc_goto_1'] = function(uid, value)
                uidExecute(uid, [=[
                    addTrigger(SYS_ON_KILL, function(monsterID)
                        if getMonsterName(monsterID) == '钉耙猫' then
                            postString('已经消灭钉耙猫，任务【%%s】已经更新！')
                            sendNotify(%d, %d, %d, %%d)
                            return true
                        end
                    end)
                ]=], questName, uid)

                uidPostXML(uid, questPath, [=[
                    <layout>
                        <par>这是一个测试。</par>
                        <par>杀死一只钉耙猫，任务<t color="red">【%%s】</t>将进入下一步。</par>
                        <par></par>
                        <par><event id="%%s">退出</event></par>
                    </layout>
                ]=], questName, SYS_EXIT)
            end
        })
    ]],

    getQuestName(),
    getUID(), getTLSTable().threadKey, getTLSTable().threadSeqID)

    local notifies = waitNotify(1)
    local playerUID = notifies[1][1]

    uidExecute(playerUID,
    [[
        postString('任务协程确认：任务【%s】被%s恢复')
    ]],

    getQuestName(),
    uidExecute(playerUID, [[ return getName() ]]));
end
