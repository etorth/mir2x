setNPCLook(23)
setNPCGLoc(412, 133)
setNPCSell({
    '青铜头盔',
    '魔法头盔',
    '布衣（男）',
    '布衣（女）',
    '轻型盔甲（男）',
    '轻型盔甲（女）',
})

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我不愿意和你这样丧尽天良的人进行交易。</par>
                    <par></par>

                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCDONE)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>这里是沙巴克城<t color="RED">%s</t>行会的领地。</par>
                    <par>你需要什么？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>防御工具</par>
                    <par><event id="npc_goto_2">出售</event>防御工具</par>
                    <par><event id="npc_goto_3">修理</event>防御工具</par>
                    <par><event id="npc_goto_4">对今日的任务进行了解</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_NPCDONE)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>来，需要吗？挑一下吧。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
        uidPostSell(uid)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要卖的衣服（头盔）放到上面。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>确实要修理吗？</par>
                <par></par>

                <par><event id="npc_goto_5">修理</event></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_4"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    ["npc_goto_5"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要修理的衣服（头盔）放上来，嗯，东西弄得很脏啊。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,


    ["npc_goto_6"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修得不错。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,
}
