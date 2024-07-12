_G.minQuestLevel = 3

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '听了客栈店员的苦衷，去找喜欢占便宜的洪气霖吧。'}
        setupNPCQuestBehavior('比奇县_0', '客栈店员_1', uid,
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
                            <par>遇到那个客人了吗？</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

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
                            <par>呃...什么？呃啊呃啊...是不是来这儿嘲弄我来了？呃...</par>
                            <par></par>
                            <par><event id="npc_start">我受旅馆主人之托而来，听说您在这儿白吃白住了一个多月吧？</event></par>
                            <par><event id="npc_abort">看你醉醺醺的样子，简直就没法儿说话。我还是走吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_start = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊？就那件事儿？呃...又不是我有钱不想给，我只是没钱而已...</par>
                            <par></par>
                            <par><event id="npc_criticize_only">要么去干活偿还，要么就去乞讨来支付住宿费。</event></par>
                            <par><event id="npc_pay_on_behalf">虽然我不知道到底是怎么回事儿，不过你欠下住宿费就由我来付吧！下次可不要再去麻烦别人了啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_abort = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是啊，滚！叫你滚啊！ 呃...全给我滚开！呼...呃...</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_criticize_only = function(uid, args)
                    qstapi.setState(questUID, {uid=uid, state='quest_criticize_only', exitfunc=string.format([=[ runNPCEventHandler(%d, %d, {SYS_EPUID, %s}, SYS_ENTER) ]=], getUID(), uid, asInitString(questName))})
                end,

                npc_pay_on_behalf = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呃...真是太感谢了！我落的如此惨状，过去我也曾是堂堂的商坛主人呢！我不能如此厚颜地接受别人的帮助...请收下这个吧！只要看到这个，几个还记得我的比奇省商人们会照应你的！</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_pay_on_behalf'})
                end,
            }
        ]])
    end,

    quest_pay_on_behalf = function(uid, args)
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
                            <par>虽然我现在落的如此窘境...您却还给我留下最后的自尊，多谢了！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '客栈店员_1', uid,
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
                            <par>见到那个客人了吗？</par>
                            <par><event id="npc_pay_on_behalf">已经把话跟他转达了，另外欠下的住宿费我来支付吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_pay_on_behalf = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯...真是近来少见的善心人啊！住宿费一共是1000钱。</par>
                            <par><event id="npc_give_gift">给您！</event></par>
                        </layout>
                    ]=])
                end,

                npc_give_gift = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>谢谢啦！还有这个略表一下我的谢意吧！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    quest_criticize_only = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '客栈店员_1', uid,
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
                            <par>见到那个客人了吗？</par>
                            <par><event id="npc_criticize_only">已经把话转达给他了。</event></par>
                        </layout>
                    ]=])
                end,

                npc_criticize_only = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是吗...那个人要是自觉的话现在应该已经离开旅馆了。那些欠下的住宿费就算了吧！</par>
                            <par>谢谢啦！还有这个略表一下我的谢意吧！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '客栈店员_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if plyapi.getLevel(uid) < minQuestLevel then
                return false
            end

            return qstapi.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>唉...真是担心啊！论人情吧！又不能把他赶走。要是谁来替我让那个客人走就好了...</par>
                    <par></par>
                    <par><event id="npc_ask">什么事啊？</event></par>
                </layout>
            ]=])
        end,

        npc_ask = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>啊！这位侠客，拜托您一件事。有个客人在我们旅馆白吃白住了一个多月，您能不能先替他垫上这笔钱或者干脆帮我把他赶出去呢？</par>
                    <par></par>
                    <par><event id="npc_accept">让我跟他说说吧！</event></par>
                    <par><event id="npc_refuse">我实在是没这个闲工夫啊！</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那就太谢谢了！那个客人白天时一般在酒摊儿附近喝的烂醉！</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>是吗？嗯...这真是郁闷啊，真愁人啊！</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])
