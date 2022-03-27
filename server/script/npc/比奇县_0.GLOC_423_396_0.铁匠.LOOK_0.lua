setNPCSell({'破山剑', '旋风流星刀', '破魂'})

local function randomHeadString()
    if math.random(0, 1) == 0 then
        return '欢迎来到大城市比奇省！'
    else
        return '有什么可以为你效劳的吗？'
    end
end

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，%s<emoji id="0"/></par>
                <par></par>
                <par><event id="event_post_sell">购买武器</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), randomHeadString(), SYS_NPCDONE))
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid)
    end,
}
