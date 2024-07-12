_G.minQuestLevel = 11

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        uidRemoteCall(uid, uid, getUID(),
        [[
            local playerUID, questUID = ...
            addTrigger(SYS_ON_GAINITEM, function(itemID, seqID)
                if hasItem(getItemID('铁矿'), 0, 5) then
                    postString('已经收集到5块铁矿了，快回去找怡美吧！')
                    qstapi.setState(questUID, {uid=playerUID, state='quest_got_iron'})
                    return true
                end
            end)
        ]])

        setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有带来我要的铁矿啊！</par>
                            <par></par>
                            <par><event id="npc_where_to_get_iron">去哪儿才能找到铁矿呢？</event></par>
                            <par><event id="npc_patience">请再给我一些时间。</event></par>
                        </layout>
                    ]=])
                end,

                npc_where_to_get_iron = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不知道就说不知道嘛！嗨...</par>
                            <par>先去武器店或铁匠铺买把鹤嘴锄，再去矿山就可以挖到各种矿石。</par>
                            <par>从中挑出5个纯度在13以上的铁矿带给我就行。</par>
                            <par>离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_patience = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那我等你的好消息。</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    quest_got_iron = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>拿来铁矿了啊！嘻嘻，现在不用担心原料不足，可以稳定的供应顾客的需求啦！</par>
                            <par>太谢谢你啦！这是我说好为你定做的特制轻型盔甲。但愿它够带给你很大帮助，呵呵呵！</par>
                            <par>这可是我特地为您精心制作的哦，所以要像我一样珍惜爱护它啊，呵呵呵！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '怡美_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return qstapi.getState(questUID, {uid=uid}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if plyapi.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                        <par>尽管你和王大人的关系十分熟，但是我跟你这种等级没有达到%d的毛头小子没什么话可说。想让我完全信任你的话还是再去好好修炼一下再来找我吧！</par>
                        <par></par>
                        <par><event id="%s">结束</event></par>
                    </layout>
                ]=], plyapi.getName(uid), minQuestLevel, SYS_EXIT)

            else
                local dressName = getItemName((plyapi.getWLItem(uid, WLG_DRESS) or {}).itemID)
                if not dressName then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>不过...你这样的侠客，却不穿衣服在这里招摇过市真的好吗？这种打扮实在是让我难以置信啊！</par>
                            <par>本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="npc_ask_when_not_interested">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid))

                elseif string.match(dressName, '布衣.+') then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>不过...看起来你今天的穿着很是稀松平常，说实话这种打扮实在难以让我信任啊！</par>
                            <par>不知你是否知道我们店里所卖的轻型盔甲呢？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="npc_ask_when_not_interested">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid))

                elseif string.match(dressName, '轻型盔甲.+') then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>噢！已经穿上轻型盔甲了！的确与众不同啊！不过这衣服好像还是有点太平常了！</par>
                            <par>实不相瞒，我有一件关于这种盔甲的事情要拜托你，如果你能够帮我我一把话，我就会为你做一套更漂亮的轻型盔甲。</par>
                            <par><event id="npc_introduce_quest">有什么要拜托的事情请您尽管说。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid))

                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>您穿上这身衣服果然是器宇不凡！</par>
                            <par>本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="npc_ask_when_not_interested">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid))
                end
            end
        end,

        npc_query = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par><t color="red">轻型盔甲</t>是等级达到11级之后才可以穿上的防御服。</par>
                    <par>主要部分都是用钢铁打造的，所以比起布衣要重的多，但是防御力也特别的好。</par>
                    <par>实不相瞒，我有一件关于这种盔甲的事情要拜托你...</par>
                    <par><event id="npc_introduce_quest">有什么要拜托的事情请您尽管说。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_ask_when_not_interested = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>既然您不感兴趣，那我就不再介绍了...</par>
                    <par>其实我并非想向您推销轻型盔，而是有一件关于这种盔甲的事情要拜托你...</par>
                    <par><event id="npc_introduce_quest">有什么要拜托的事情请您尽管说。</event></par>
                </layout>
            ]=], plyapi.getName(uid))
        end,

        npc_introduce_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那就太谢谢了！是这样的，最近来买轻型盔甲的顾客非常多，可是用来做辅强剂的铁矿不够用了。</par>
                    <par>所以您要是能够给我找来5个纯度13以上的铁矿的话，我就会为你特别制作一套轻型盔甲。</par>
                    <par></par>
                    <par><event id="npc_where_to_get_iron">到哪儿去找铁矿呢？</event></par>
                    <par><event id="npc_accept_quest">知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_where_to_get_iron = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>先去武器店或铁匠铺买把鹤嘴锄，再去<event id="npc_nearest_iron">矿山</event>就可以挖到各种矿石！</par>
                    <par>挑出我所需要的纯度在13以上的铁矿之后，剩下的还可以卖给武器店赚到很多钱。</par>
                    <par>如果你觉得麻烦不想去矿山挖矿的话也可以去武器店购买铁矿，但是那样的话就有点亏本哦！</par>
                    <par><event id="npc_accept_quest">知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_nearest_iron = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。</par>
                    <par></par>
                    <par><event id="npc_accept_quest">知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept_quest = function(uid, args)
            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
