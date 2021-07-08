setNPCLook(18)
setNPCGLoc(396, 116)

local invop = require('npc.include.invop')
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这里是沙巴克城<t color="RED">%s</t>行会的领地。</par>
                <par>贫道是仓库管理员，需要的话，可以把东西暂时寄存在我这里。</par>
                <par></par>

                <par><event id="npc_goto_secure">寄存</event>物品</par>
                <par><event id="npc_goto_get_back">取回</event>物品</par>
                <par><event id="npc_goto_set_password">设置</event>仓库密码</par>
                <par><event id="npc_goto_daily_quest">对今日的任务进行了解</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], getSubukGuildName(), SYS_NPCDONE)
    end,

    ["npc_goto_secure"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你想寄存什么东西？</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_get_back"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你想找什么，看了目录以后再决定吧。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_set_password"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请设置你的仓库密码。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
        invop.postStartInput(uid, '<par>请输入密码</par>', 'npc_goto_get_set_password_1', true)
    end,

    ["npc_goto_get_set_password_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你输入的密码是：%s</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], value, SYS_NPCDONE)
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
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
