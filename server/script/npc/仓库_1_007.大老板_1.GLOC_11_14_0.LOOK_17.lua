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
                    <par>这里寄存和出售道馆里使用的东西。</par>
                    <par></par>
                    <par><event id="npc_goto_buy" >购买</event>物品</par>
                    <par><event id="npc_goto_sell">出售</event>物品</par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_buy"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>有什么需要的尽管挑。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_sell"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把不用的东西卖给我，我给你个合理的价钱。</par>
                <par></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
