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
            local dialog = require('include.dialog')
            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, '你是来测试脚本的吗？',
                    {
                        dialog.link('npc_done_test', '完成测试', {close = true}),
                        dialog.link(SYS_EXIT, '退出'),
                    })

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
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, value)
            dialog.post(uid, questPath,
            {
                '你好？',
                '我可以帮你测试脚本功能。',
            },
            {
                dialog.link('npc_test_script', '测试脚本'),
                dialog.link('npc_test_deliver_iterms', '测试邮寄'),
                dialog.link('npc_test_switch_map', '测试地图切换', {close = true, args = '{\'比奇县_0\',390,400}'}),
                dialog.link('npc_test_random_move', '狂奔', {close = true}),
                getNPCMapLocXML("event", {id="npc_test_switch_map", close="1"}),
                dialog.link(SYS_EXIT, '退出'),
            })
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

            server.player.addItem(uid, '制魔宝玉')
        end,

        npc_test_random_move = function(uid, value)
            local i = 1
            while i < 10000 do
                local done, firstRes = pcall(uidRemoteCall, uid, [=[ return randomMove() ]=])
                if done then
                    if firstRes ~= nil then
                        i = i + 1
                        pause(200)
                    end
                else
                    addLog(LOGTYPE_WARNING, 'randomMove(%d) failed: %s', uid, firstRes)
                    break
                end
            end
        end,

        npc_test_deliver_iterms = function(uid, value)
            uidRemoteCall(uid,
            [=[
                deliverItem(getItemID(SYS_GOLDNAME), 1000)
            ]=])
        end,
    })
]])
