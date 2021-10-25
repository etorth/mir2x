setNPCLook(18)
setNPCGLoc(396, 116)

local invop = require('npc.include.invop')
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这里是沙巴克城<t color="red">%s</t>行会的领地。</par>
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
        invop.uidStartSecure(uid, "npc_goto_secure_query", "npc_goto_secure_commit", {'恢复药水', '武器', '药粉', '传送卷轴', '技能书', '护身符', '头盔', '戒指', '手镯', '项链', '衣服', '鞋'})
    end,

    ["npc_goto_secure_query"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这个我可以帮你保存，但要收你<t color="red">20金币</t>保管费。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_NPCINIT)
    end,

    ["npc_goto_secure_commit"] = function(uid, value)
        itemID, seqID = invop.parseItemString(value)
        uidSecureItem(uid, itemID, seqID)

        uidPostXML(uid,
        [[
            <layout>
                <par>已经放好了。</par>
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
        uidShowSecuredItemList(uid)
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
        invop.postStartInput(uid, '<layout><par>请输入密码</par></layout>', 'npc_goto_get_set_password_1', true)
    end,

    ["npc_goto_get_set_password_1"] = function(uid, value)
        tls = getCallStackTable()
        tls['set_password_1'] = value

        uidPostXML(uid,
        [[
            <layout>
                <par>请再次输入密码确认。</par>
                <par></par>

                <par><event id="%s">关闭</event></par>
            </layout>
        ]], value, SYS_NPCDONE)
        invop.postStartInput(uid, '<layout><par>请确认密码</par></layout>', 'npc_goto_get_set_password_2', true)
    end,

    ["npc_goto_get_set_password_2"] = function(uid, value)
        tls = getCallStackTable()
        firstInput = tls['set_password_1']

        -- TODO this won't work, just keep the wrong code here
        -- because every npc_goto uses a new main(uid) call, thus a fresh call stack

        if firstInput == value then
            uidPostXML(uid,
            [[
                <layout>
                    <par>设置密码成功！</par>
                    <par></par>

                    <par><event id="%s">前一步</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], SYS_NPCINIT, SYS_NPCDONE)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>两次密码输入不一致，设置密码失败。</par>
                    <par></par>

                    <par><event id="%s">设置密码</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], "npc_goto_set_password", SYS_NPCDONE)
        end
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
