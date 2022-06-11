setEventHandler(
{
    [SYS_NPCINIT] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我不想和你这种坏人做生意。</par>
                    <par></par>

                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>欢迎光临，有什么事吗？</par>
                    <par></par>

                    <par><event id="npc_goto_1">修理</event>武器</par>
                    <par><event id="npc_goto_2">特殊修理</event>武器</par>
                    <par><event id="npc_goto_3">对今日的任务进行了解</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_NPCDONE)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要修理的武器拿来。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>我不会。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,
})
