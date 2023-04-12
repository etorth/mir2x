setNPCSell({
    '金创药（小）',
    '魔法药（小）',
    '金创药（中）',
    '魔法药（中）',
    '金创药（大）',
    '魔法药（大）',
    '金创药（特）',
    '魔法药（特）',
    '太阳水',
})

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
                    <par>我是在这里学习药学的研修生，你需要什么东西？</par>
                    <par></par>

                    <par><event id="npc_goto_1">购买</event>药品</par>
                    <par><event id="npc_goto_2">出售</event>药品</par>
                    <par><event id="npc_goto_3">对今日的任务进行了解</event></par>
                    <par><event id="%s">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请挑选你所需要的药和用量。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        uidPostSell(uid)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把你想出售的药放在这里。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
    end,

    ["npc_goto_3"] = function(uid, value)
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
