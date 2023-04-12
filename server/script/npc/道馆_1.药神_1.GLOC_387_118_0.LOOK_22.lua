setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我不愿意和你这样丧尽天良的人进行交易。</par>
                    <par></par>

                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_EXIT)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>这里是沙巴克城<t color="RED">%s</t>行会的领地。</par>
                    <par>嗯，我在这里进行试验，制造各类药粉，同时也传授知识。你需要什么？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>药粉</par>
                    <par><event id="npc_goto_2">对今日的任务进行了解</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请选择你想购买的药粉。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_2"] = function(uid, value)
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
