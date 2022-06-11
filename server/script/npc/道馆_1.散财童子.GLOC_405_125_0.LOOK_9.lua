setEventHandler(
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，欢迎领取礼物！<emoji id="0"/></par>
                <par></par>
                <par><event id="npc_goto_1">领取金币</event></par>
                <par><event id="npc_goto_2">领取装备</event></par>
                <par><event id="npc_goto_random_move_%s">随机行走100步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_NPCDONE, SYS_NPCDONE))
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
            ]], SYS_NPCINIT, SYS_NPCDONE)
        end
    end,

    ["npc_goto_2"] = function(uid, value)
        uidGrant(uid, '斩马刀', 2)
        uidGrant(uid, '五彩鞋', 1)
        uidGrant(uid, '井中月', 1)
    end,

    ["npc_goto_random_move_" .. SYS_NPCDONE] = function(uid, value)
        uidExecute(uid, [[
            local i = 1
            while i < 100 do
                if randomMove() then
                    i = i + 1
                    pause(1000)
                end
            end
        ]])
    end,
})
