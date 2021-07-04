setNPCLook(18)
setNPCGLoc(396, 116)

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这里是沙巴克城<t color="RED">%s</t>行会的领地。</par>
                <par>贫道是仓库管理员，需要的话，可以把东西暂时寄存在我这里。</par>
                <par></par>

                <par><event id="npc_goto_1">寄存</event>物品</par>
                <par><event id="npc_goto_2">取回</event>物品</par>
                <par><event id="npc_goto_3">设置</event>仓库密码</par>
                <par><event id="npc_goto_4">对今日的任务进行了解</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], getSubukGuildName(), SYS_NPCDONE)
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你想寄存什么东西？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你想找什么，看了目录以后再决定吧。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>尚未支持。。。</par>
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
}
