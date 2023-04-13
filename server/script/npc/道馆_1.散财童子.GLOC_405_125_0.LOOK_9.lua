setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，欢迎领取礼物！<emoji id="0"/></par>
                <par></par>
                <par><event id="npc_goto_1">领取金币</event></par>
                <par><event id="npc_goto_2">领取装备</event></par>
                <par><event id="npc_goto_random_move:%s">随机行走100步</event></par>
                <par><event id="npc_goto_test_player_trigger">测试任务触发器</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_EXIT, SYS_EXIT))
    end,

    ["npc_goto_1"] = function(uid, value)
        local currTime = getAbsTime()
        local lastTime = argDefault(uidDBGetKey(uid, 'fld_time'), 0)

        if currTime > lastTime + 10 then
            uidGrantGold(uid, 1000000)
            uidDBSetKey(uid, 'fld_time', currTime)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>你刚刚领取过金币了，请稍后再来！<emoji id="1"/></par>
                    <par></par>
                    <par><event id="%s">返回</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_ENTER, SYS_EXIT)
        end
    end,

    ["npc_goto_2"] = function(uid, value)
        uidGrant(uid, '斩马刀', 2)
        uidGrant(uid, '五彩鞋', 1)
        uidGrant(uid, '井中月', 1)
    end,

    ["npc_goto_random_move:" .. SYS_EXIT] = function(uid, value)
        -- don't send a loop of randMove to player
        -- randMove() can fail without yielding, which makes a dead loop in player script runner

        local i = 1
        while i < 100 do
            local done, firstRes = pcall(uidExecute, uid, [[ return randomMove() ]])
            if done then
                if firstRes ~= nil then
                    i = i + 1
                    pause(1000)
                end
            else
                addLog(LOGTYPE_WARNING, 'randomMove(%d) failed: %s', uid, firstRes)
                break
            end
        end
    end,

    ["npc_goto_test_player_trigger"] = function(uid, value)
        local maxTriggerTime = 10
        local firstTimeAdded = uidExecute(uid, [[
            if _G.RSVD_NAME_player_trigger_added then
                return false
            else
                local killedCount = 0
                local killedMaxCount = %d

                addTrigger(SYS_ON_KILL, function(monsterID)
                    killedCount = killedCount + 1
                    postString('测试任务触发器：【%s】：%%s杀死了%%s，当前共杀死了%%d只怪物。', getName(), getMonsterName(monsterID), killedCount)

                    if killedCount < killedMaxCount then
                        return false
                    else
                        postString('测试任务触发器：【%s】：触发器已经触发%%d次，自动解除触发。', killedCount)
                        _G.RSVD_NAME_player_trigger_added = nil
                        return true
                    end
                end)

                _G.RSVD_NAME_player_trigger_added = true
                return true
            end
        ]], maxTriggerTime, getNPCName(), getNPCName())

        if firstTimeAdded then
            uidPostXML(uid,
            [[
                <layout>
                    <par>尝试杀死一只怪物，测试是否触发系统消息！此消息触发<t color="RED">%d</t>次后自动解除。<emoji id="2"/></par>
                    <par></par>
                    <par><event id="%s">返回</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], maxTriggerTime, SYS_ENTER, SYS_EXIT)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>你已经安装了任务触发器。</par>
                    <par></par>
                    <par><event id="%s">返回</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_ENTER, SYS_EXIT)
        end
    end,
})
