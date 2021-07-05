setNPCLook(15)
setNPCGLoc(426, 117)
setNPCSell({
    '木剑',
    '匕首',
    '青铜剑',
    '铁剑',
    '乌木剑',
    '半月',
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
                    <par>这里是道馆寄存武器的地方，你需要什么武器吗？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>武器</par>
                    <par><event id="npc_goto_2">出售</event>武器</par>
                    <par><event id="npc_goto_3">修理</event>武器</par>
                    <par><event id="npc_goto_4">特殊修理</event>武器</par>
                    <par><event id="npc_goto_5">对今日的任务进行了解</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_NPCDONE)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>各种武器在这里保存得很好。</par>
                <par>你想要什么武器？</par>
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
                <par>你有想卖掉的武器？</par>
                <par>让我看看。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要修理的武器放上去。</par>
                <par></par>

                <par><event id="npc_goto_6">修理</event></par>
            </layout>
        ]], SYS_NPCINIT)
        uidPostRepair(uid, "npc_goto_6", {'武器'})
    end,

    ["npc_goto_4"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修理费用是%d。</par>
                <par>这个特修是只有带着物品的情况下才可以修理。请确认一下是否带着。</par>
                <par></par>

                <par><event id="npc_goto_8">修理</event></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], 0, SYS_NPCINIT)
    end,

    ["npc_goto_5"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    ["npc_goto_6"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你的武器太旧了，请稍等。</par>
                <par>请选择你要修理的武器。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_7"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修理完毕。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_8"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>修理得很好。</par>
                <par>修理费用是%d金币。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], 0, SYS_NPCINIT)
    end,
}
