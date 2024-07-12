setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我不想和你这种坏人做生意。</par>
                    <par></par>

                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_EXIT)

        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>很高兴见到你，有什么事吗？</par>
                    <par></par>
                    <par><event id="npc_goto_special_repair">特殊修理</event>武器</par>
                    <par><event id="npc_goto_unequip_weapon">请求把剑从手分离开</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_EXIT)
        end
    end,

    ["npc_goto_special_repair"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>我不会。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_unequip_weapon"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>我不会。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
