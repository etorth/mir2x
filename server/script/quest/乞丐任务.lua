local minQuestLevel = 3

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
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '遇到那个客人了吗？',
                    dialog.link(SYS_EXIT, '结束'))
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
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '呃<t wrap="0">···</t>什么？呃啊呃啊<t wrap="0">···</t>是不是来这儿嘲弄我来了？呃<t wrap="0">···</t>',
                    {
                        dialog.link('npc_start', '我受旅馆主人之托而来，听说您在这儿白吃白住了一个多月吧？'),
                        dialog.link('npc_abort', '看你醉醺醺的样子，简直就没法儿说话。我还是走吧！'),
                    })
                end,

                npc_start = function(uid, args)
                    dialog.post(uid, questPath, '啊？就那件事儿？呃<t wrap="0">···</t>又不是我有钱不想给，我只是没钱而已<t wrap="0">···</t>',
                    {
                        dialog.link('npc_criticize_only', '要么去干活偿还，要么就去乞讨来支付住宿费。'),
                        dialog.link('npc_pay_on_behalf', '虽然我不知道到底是怎么回事儿，不过你欠下住宿费就由我来付吧！下次可不要再去麻烦别人了啊！'),
                    })
                end,

                npc_abort = function(uid, args)
                    dialog.post(uid, questPath, '是啊，滚！叫你滚啊！呃<t wrap="0">···</t>全给我滚开！呼<t wrap="0">···</t>呃<t wrap="0">···</t>',
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_criticize_only = function(uid, args)
                    server.quest.setState(questUID, {uid=uid, from=SYS_ENTER, state='quest_criticize_only', exitfunc=string.format([=[ runNPCEventHandler(%d, %d, {SYS_EPUID, %s}, SYS_ENTER) ]=], getUID(), uid, asInitString(questName))})
                end,

                npc_pay_on_behalf = function(uid, args)
                    if not server.quest.setState(questUID, {uid=uid, from=SYS_ENTER, state='quest_pay_on_behalf'}) then
                        return
                    end

                    server.player.addItem(uid, '气霖证书', 1)

                    dialog.post(uid, questPath, '呃<t wrap="0">···</t>真是太感谢了！我落得如此惨状，过去我也曾是堂堂的商坛主人呢！我不能如此厚颜地接受别人的帮助<t wrap="0">···</t>请收下这个吧！只要看到这个，几个还记得我的比奇省商人们会照应你的！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    quest_pay_on_behalf = function(uid, args)
        setQuestDesp{uid=uid, '已与洪气霖进行对话，去找客栈店员替洪气霖支付住宿费。'}

        setupNPCQuestBehavior('比奇县_0', '洪气霖_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '虽然我现在落得如此窘境<t wrap="0">···</t>您却还给我留下最后的自尊，多谢了！',
                    dialog.link(SYS_EXIT, '退出'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '客栈店员_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '见到那个客人了吗？',
                    dialog.link('npc_pay_on_behalf', '已经把话跟他转达了，另外欠下的住宿费我来支付吧！'))
                end,

                npc_pay_on_behalf = function(uid, args)
                    dialog.post(uid, questPath, '嗯<t wrap="0">···</t>真是近来少见的善心人啊！住宿费一共是1000钱。',
                    server.player.getGold(uid) >= 1000
                        and dialog.link('npc_give_gift', '给您！')
                        or  dialog.link('npc_pay_later', '身上钱不够，下次我来的时候再付给您吧！'))
                end,

                npc_pay_later = function(uid, args)
                    dialog.post(uid, questPath, '这样啊？好吧！那么下次再会！',
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_give_gift = function(uid, args)
                    if server.quest.getState(questUID, {uid=uid}) ~= 'quest_pay_on_behalf' then
                        return
                    end

                    if not server.player.removeGold(uid, 1000) then
                        dialog.post(uid, questPath, '住宿费一共是1000钱，你带的钱不够。',
                        dialog.link('npc_pay_later', '下次我来的时候再付给您吧！'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_pay_on_behalf', state=SYS_DONE}) then
                        server.player.addItem(uid, SYS_GOLDNAME, 1000)
                        return
                    end

                    server.quest.setDesp(questUID, {uid=uid, '告诉客栈店员已完成任务，替洪气霖支付了赊账的住宿费。'})
                    server.player.dbAddFlag(uid, 'done_quest_乞丐任务_pay_on_behalf')
                    server.player.addItem(uid, '银手镯', 1)

                    dialog.post(uid, questPath, '谢谢啦！还有这个略表一下我的谢意吧！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    quest_criticize_only = function(uid, args)
        setQuestDesp{uid=uid, '已与洪气霖进行谈话，请告诉客栈店员你已与洪气霖进行谈话。'}

        setupNPCQuestBehavior('比奇县_0', '洪气霖_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '这<t wrap="0">···</t>这个无情的世界啊！有了人才有钱，有了钱才有人！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '客栈店员_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '见到那个客人了吗？',
                    dialog.link('npc_criticize_only', '已经把话转达给他了。'))
                end,

                npc_criticize_only = function(uid, args)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_criticize_only', state=SYS_DONE}) then
                        return
                    end

                    server.quest.setDesp(questUID, {uid=uid, '告诉客栈店员已经完成任务。'})
                    server.player.addItem(uid, '耐久铁手镯', 1)

                    dialog.post(uid, questPath, '是吗<t wrap="0">···</t>那个人要是自觉的话现在应该已经离开旅馆了。那些欠下的住宿费就算了吧！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '客栈店员_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if server.player.getLevel(uid) < minQuestLevel then
                return false
            end

            return server.quest.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            dialog.post(uid, questPath, '唉<t wrap="0">···</t>真是担心啊！论人情吧！又不能把他赶走。要是谁来替我让那个客人走就好了<t wrap="0">···</t>',
            dialog.link('npc_ask', '什么事啊？'))
        end,

        npc_ask = function(uid, args)
            dialog.post(uid, questPath, server.player.getGender(uid)
                and '啊！这位侠客，拜托您一件事。有个客人在我们旅馆白吃白住了一个多月，您能不能先替他垫上这笔钱或者干脆帮我把他赶出去呢？'
                or  '啊！这位女侠，有件事情想拜托您。您能不能先替他垫上这笔钱或者干脆帮我把他赶出去呢？',
            {
                dialog.link('npc_accept', '让我跟他说说吧！'),
                dialog.link('npc_refuse', '我实在是没这个闲工夫啊！'),
            })
        end,

        npc_accept = function(uid, args)
            if not server.quest.setState(questUID, {uid=uid, from=SYS_LUANIL, state=SYS_ENTER}) then
                return
            end

            dialog.post(uid, questPath, '那就太谢谢了！那个客人白天时一般在酒摊儿附近喝得烂醉！',
            dialog.link(SYS_EXIT, '结束'))
        end,

        npc_refuse = function(uid, args)
            dialog.post(uid, questPath, '是吗？嗯<t wrap="0">···</t>这真是郁闷啊，真愁人啊！',
            dialog.link(SYS_EXIT, '结束'))
        end,
    })
]])
