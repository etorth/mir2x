setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>你是来卖肉的？</par>
                <par></par>
                <par><event id="npc_goto_sell">卖</event>肉</par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_EXIT)
    end,

    ["npc_goto_sell"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>高价收购优质肉。</par>
                <par>沾上土的或被火烧过的肉廉价收购。</par>
                <par></par>
                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_ENTER, SYS_EXIT)
    end,
})
