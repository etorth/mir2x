function main()
    uidRemoteCall(getNPCharUID('道馆_1', '物品展示商人'), getUID(), getQuestName(),
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPQST, questName}

        setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你好？</par>
                        <par>我可以帮你测试脚本功能。</par>
                        <par></par>
                        <par><event id="npc_test_script">测试脚本</event></par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_test_script = function(uid, value)
                uidRemoteCall(questUID, uid, [=[ setUIDQuestState(..., SYS_ENTER)]=])
            end
        })
    ]])

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            setupNPCQuestBehavior('道馆_1', '物品展示商人', uid,
            [[
                return getUID(), getQuestName()
            ]],
            [[
                local questUID, questName = ...
                local questPath = {SYS_EPUID, questName}
                return
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你是来测试脚本的吗？</par>
                                <par></par>
                                <par><event id="npc_done_test">完成测试</event></par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,

                    npc_done_test = function(uid, value)
                        uidRemoteCall(questUID, uid, [=[ setUIDQuestState(..., SYS_DONE) ]=])
                    end,
                }
            ]])
        end,
    })
end
