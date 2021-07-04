setNPCLook(18)
setNPCGLoc(418, 96)
setNPCSell({
    '治愈术',
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
                    <par>欢迎光临，你来买练武功的技能书籍？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>图书</par>
                    <par><event id="npc_goto_2">出售</event>图书</par>
                    <par><event id="npc_goto_3">聆听</event>关于技能书籍的说明</par>
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
                <par>请挑选你想要的书。</par>
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
                <par>请把要出售的书籍拿上来。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你想听哪类书的介绍？</par>
                <par>道士可以学习<event id="npc_goto_5">治愈术</event>，<event id="npc_goto_6">精神力战法</event>和<event id="npc_goto_7">施毒术</event>。</par>
                <par></par>

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
                <par>等级为7时可以修炼<t color="RED">治愈术</t>的第1阶段，等级为11时修炼第2阶段，16级时可以完成第3阶段的修炼。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_6"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>等级为8时可以修炼<t color="RED">精神力战法</t>的第1阶段，等级为10时修炼第2阶段，12级时可以完成第3阶段的修炼。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_7"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>等级为12时可以修炼<t color="RED">施毒术</t>的第1阶段，等级为14时修炼第2阶段，16级时可以完成第3阶段的修炼。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,
}
