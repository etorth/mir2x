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
                            <par><event id="npc_done_test" close="1">完成测试</event></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '请选择是否要进行脚本测试。'}
                    ]=])
                end,

                npc_done_test = function(uid, value)
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
                    <par><event id="npc_test_switch_map" args="{'比奇县_0',390,400}" close="1">测试地图切换</event></par>
                    <par>%s</par>
                    <par><event id="%s">退出</event></par>
                </layout>
            ]=], getNPCMapLocXML("event", {id="npc_test_switch_map", close="1"}), SYS_EXIT)
        end,

        npc_test_script = function(uid, value)
            uidRemoteCall(questUID, uid, questName,
            [=[
                local playerUID, questName = ...
                setQuestState{uid=playerUID, state=SYS_ENTER, exitfunc=function()
                    runNPCEventHandler(getNPCharUID('道馆_1', '物品展示商人'), playerUID, {SYS_EPUID, questName}, SYS_ENTER)
                end}
            ]=])
        end,

        npc_test_switch_map = function(uid, value)
            uidRemoteCall(uid, value,
            [=[
                local dstStr = ...
                spaceMove(load('return' .. dstStr)())
            ]=])

            plyapi.addItem(uid, '制魔宝玉')
        end,
    })
]])
