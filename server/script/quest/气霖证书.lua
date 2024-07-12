setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '世玉_1', uid,
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
                            <par>对不起，本店不接受这种证书。</par>
                            <par></par>
                            <par><event id="npc_ask">为什么？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>看来你不了解给你证书的人。洪气霖一生坎坷，他家经营的店铺原本在比奇县一带很有名望，但自从他们家的货船失事以后，他们家族就没落了。虽然我知道洪家的处境很艰难，但我们也不能因此而蒙受损失啊，所以我们不能接受这种证书。</par>
                            <par></par>
                            <par><event id="npc_who_accept">那这证书怎么办？</event></par>
                        </layout>
                    ]=])
                end,

                npc_who_accept = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯...首饰店所蒙受的损失少一点，说不定他们会接受。</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_ask_jewelry'})
                end,
            }
        ]])
    end,

    quest_ask_jewelry = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '恩实_1', uid,
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
                            <par>嗯...这个是洪气霖那个人的证书啊！对不起，我们也不能收下这证书！</par>
                            <par>那么你知道关于洪气霖这个人的事儿吗？真是越想越觉得蹊跷！抛弃好好的家，过着四处流浪的生活。好不容易遇到知己，结为百年好合...但是因为这家伙是土匪和妻子分手了。两个人分手的时候约定在比奇省这儿见面，于是就遵照约定这样一直在这儿等下去...啧啧...</par>
                            <par></par>
                            <par><event id="npc_ask_more">那么还知道关于这个人其他的什么事儿吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_more = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这个...我知道的就只有这些了。唉...人世艰辛啊！前不久我们店里也来过一个失魂落魄的女子，据说她在逃难时失去了丈夫要靠自己来混口饭吃。那个人好像有什么难言之隐，一直少言寡语。我们商店因为人手够，所以介绍她去棉布店工作了。不过依我看那个女子好像和洪气霖是从一个地方来的！</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_ask_wife'})
                end,
            }
        ]])
    end,

    quest_ask_wife = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '洪气霖_1', uid,
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
                            <par>听说棉布店有一个女子和我是同乡？难道...不会的，这是不可能的！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '苏百花_1', uid,
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
                            <par>来找我有什么事儿吗？哦？这证书的字体？！</par>
                            <par></par>
                            <par><event id="npc_ask">您见过的字体吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这和我丈夫的字体一模一样啊！拜托了，请你告诉我，这证书从哪得来的呢？</par>
                            <par></par>
                            <par><event id="npc_where_from">酒店附近的一个叫做洪气霖的人那儿得来的...</event></par>
                        </layout>
                    ]=])
                end,

                npc_where_from = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你说那个人姓洪名奇莲？啊...他还活着啊！自从流离失散之后，虽然觉得很难活下去...但一直认为只要还活着的话总会有一天能见上一面的，所以一直在这儿苦苦等候...终于没有白等啊！</par>
                            <par>拜托侠客您一件事！请您把这个玉指环拿给他，告诉他苏白花还活着！并告诉他如果他依然还爱我的话，就让他来这里接我吧！</par>
                            <par></par>
                            <par><event id="npc_ask_why_not_go_directly">为什么不直接去找他呢？</event></par>
                        </layout>
                    ]=])

                    plyapi.addItem(uid, getItemID('玉指环'), 1)
                end,

                npc_ask_why_not_go_directly = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>只能这样啊！万一他已经有了别的妻子，我就会妨碍他们的！所以请你替我去打听一下他的心意啊！</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_ask_husband'})
                end,
            }
        ]])
    end,

    quest_ask_husband = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '洪气霖_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    if plyapi.hasItem(uid, getItemID('玉指环'), 0, 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>天啊！这个玉指环不是我作为定情信物送给妻子的吗？请快快告诉我，你是从哪儿得到这个指环的？</par>
                                <par></par>
                                <par><event id="npc_where_it_is_from">是从棉布店的苏白花夫人那得到的！</event></par>
                            </layout>
                        ]=])
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>那个在棉布店工作的女子好像就是我妻子啊！哦...难道没有什么要转交给我的东西吗？</par>
                                <par></par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end,

                npc_where_it_is_from = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>白花...！也就是说白花她现在还活着！！！</par>
                            <par></par>
                            <par><event id="npc_ask_for_ring">是啊！夫人让我拿这个指环给你看，如果你还没有变心的话就让你去找她！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_for_ring = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>曾经的海誓山盟怎么能变...真不知该如何表达我内心对您的感激之情了！</par>
                            <par>啊，可是这个指环您要怎么办呢？这可是我和她的定情信物啊！能还给我吗？</par>
                            <par></par>
                            <par><event id="npc_give_ring">当然！</event></par>
                        </layout>
                    ]=])
                end,

                npc_give_ring = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是太感谢了！啊，收下这个吧！这本来是我们家族的传家之宝，但现在已经家门零落还要这传家宝又有什么用呢？别谦让，请收下吧！</par>
                            <par>我收拾一下马上就去！</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    plyapi.addItem(uid, getItemID('制魔宝玉'), 1)
                    qstapi.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '苏百花_1', uid,
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
                            <par>那么请你一定要把我给你的这个玉指环带给他看，并探查一下他的心意，多多拜托您了！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '世玉_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if not plyapi.hasItem(uid, getItemID('气霖证书'), 0, 1) then
                return false
            end

            return qstapi.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER, exitargs=table.pack(getUID(), uid, questName), exitfunc=[=[
                local npcUID, playerUID, questName = ...
                runNPCEventHandler(npcUID, playerUID, {SYS_EPUID, questName}, SYS_ENTER)
            ]=]})
        end,
    })
]])
