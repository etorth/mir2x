setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0_003', '石母_1', uid,
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
                    dialog.post(uid, questPath, '还没有找到我孩子吗？',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '老生_1', uid,
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
                    dialog.post(uid, questPath, '深更半夜的什么事啊？你来找我有什么事？',
                    {
                        dialog.link('npc_ask_where_is_kid', '我来找比奇省的一名妇人的孩子'),
                        dialog.link(SYS_EXIT, '结束'),
                    })
                end,

                npc_ask_where_is_kid = function(uid, args)
                    dialog.post(uid, questPath, '嘿，什么话？！我虽然是个做生意的，但我可不是诱拐别人家的孩子那种没头没脑的人。你如果是为这件事来找我，还是赶快走吧！',
                    dialog.link(SYS_EXIT, '结束'))

                    server.quest.setState(questUID, {uid=uid, state='quest_ask_book_store'})
                end,
            }
        ]])
    end,

    quest_ask_book_store = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '老生_1', uid,
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
                    dialog.post(uid, questPath, '嘿，你怎么还在？你要是再说那种没头没尾的话，就赶快给我走！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '店员_1', uid,
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
                    dialog.post(uid, questPath,
                    {
                        '关于那个夜市商人嘛...唔...',
                        '好像除了最近得到一个奇特的寿石之外就没有什么特别的消息了！那块寿石倒是蛮稀有的，据说是童子模样的呢！',
                    },
                    dialog.link('npc_tell_mom', '这好像真有点不着边际啊！不管还是要赶快去告诉那个妇人吧！'))
                end,

                npc_tell_mom = function(uid, args)
                    server.quest.setState(questUID, {uid=uid, state='quest_tell_mom'})
                end,
            }
        ]])
    end,

    quest_tell_mom = function(uid, args)
        setupNPCQuestBehavior('比奇县_0_003', '石母_1', uid,
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
                    dialog.post(uid, questPath, '还没有找到我孩子吗？',
                    dialog.link('npc_no_clue', '很遗憾，现在连一点线索都还没找到。'))
                end,

                npc_no_clue = function(uid, args)
                    dialog.post(uid, questPath,
                    {
                        '啊...这样啊...呜呜...',
                        '难道那个商人已经把我那可怜的孩子给卖了吗？呜呜...',
                    },
                    dialog.link('npc_tell_rumor', '不过倒是听说了一些怪异的传闻，好像那个商人最近得到了一块童子模样的寿石？'))
                end,

                npc_tell_rumor = function(uid, args)
                    dialog.post(uid, questPath, '哦？童子模样的寿石？对！那个寿石就是我孩子啊！拜托您一定要帮我找回那个寿石啊！！！',
                    dialog.link('npc_fell_strange', '这是怎么回事？说石头是自己的孩子？对不起，我有点不太明白你说的话。'))
                end,

                npc_fell_strange = function(uid, args)
                    dialog.post(uid, questPath, '现在还不能为你解释！不管怎么样，那个寿石就是我的孩子！帮我找回那个寿石我一定不忘您的大恩大德。请帮我把它找回来吧！',
                    dialog.link(SYS_EXIT, '结束'))

                    server.quest.setState(questUID, {uid=uid, state='quest_buy_kid_stone'})
                end,
            }
        ]])
    end,

    quest_buy_kid_stone = function(uid, args)
        setupNPCQuestBehavior('比奇县_0_003', '石母_1', uid,
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
                    dialog.post(uid, questPath, '还没有找到我孩子吗？',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '老生_1', uid,
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
                    dialog.post(uid, questPath, '怎么还是你！你要是不买东西就赶快走！',
                    {
                        dialog.link('npc_ask_for_purchase', '今天我是来买东西的。'),
                        dialog.link(SYS_EXIT, '结束'),
                    })
                end,

                npc_ask_for_purchase = function(uid, args)
                    dialog.post(uid, questPath,
                    {
                        '哦，原来如此。那你好好看看吧！都是稀罕玩意儿...',
                        '种类多着呢！这些东西商店里都不卖，而且一旦卖出去了，我就不会再进第二次货。你好好想想再买吧。',
                        '这些都是为了得到了又失去的人准备的...你就算现在买了也没什么用...',
                    },
                    {
                        dialog.link('npc_ask_for_purchase_kid_statue', '童子像'),
                        '',
                        dialog.link(SYS_EXIT, '结束'),
                    })
                end,

                npc_ask_for_purchase_kid_statue = function(uid, args)
                    dialog.post(uid, questPath, '那个有点困难.. 这东西很少见，我不想卖。',
                    dialog.link('npc_ask_for_kid_statue_price', '那我也要买'))
                end,

                npc_ask_for_kid_statue_price = function(uid, args)
                    dialog.post(uid, questPath, '你要真想要那个寿石，要么就出3000金币，要么拿个稀罕玩意儿来换！',
                    dialog.link('npc_check_exchange', '你看看我这有啥你感兴趣的东西？'))
                end,

                npc_check_exchange = function(uid, args)
                    if server.player.hasItem(uid, '制魔宝玉') then
                        dialog.post(uid, questPath,
                        {
                            '噢噢！这不是制魔宝玉嘛。这稀罕玩意儿你从哪儿弄来的？',
                            '真不错，那就用制魔宝玉交换吧！不过，那个童子形状的寿石你一定要小心，那里好像蕴藏了一些让人无从知晓的秘密。',
                        },
                        {
                            dialog.link('npc_exchange_by_stone', '好嘞！'),
                            dialog.link('npc_exchange_refuse', '我不想把这个给你。'),
                        })
                    else
                        dialog.post(uid, questPath, '没啥看得上的，你还是拿3000金币来还吧！',
                        dialog.link('npc_exchange_by_money', '好嘞！'))
                    end
                end,

                npc_exchange_refuse = function(uid, args)
                    dialog.post(uid, questPath, '你这个人真是的，你还是拿3000金币来还吧！',
                    dialog.link('npc_exchange_by_money', '好嘞！'))
                end,

                npc_exchange_by_money = function(uid, args)
                    dialog.post(uid, questPath, '真是大甩卖了。你到底知不知道那东西的价值啊？那玩意儿不是普通的东西，是蕴含着灵气的。 你小心点弄它吧。',
                    dialog.link(SYS_EXIT, '知道了'))

                    server.quest.setState(questUID, {uid=uid, state='quest_got_kid_statue'})
                end,
            }
        ]])
    end,

    quest_got_kid_statue = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '老生_1', uid,
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
                    dialog.post(uid, questPath, '怎么还是你！你要是不买东西就赶快走！',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0_003', '石母_1', uid,
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
                    dialog.post(uid, questPath, '啊啊，终于找回我的孩子了，真是太感谢了！这个虽然微薄，但也是我的一片心意！',
                    dialog.link(SYS_EXIT, '结束'))

                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0_003', '石母_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    local fnLeaveMap = function(uid)
        local x, y = uidRemoteCall(uid, [=[ return getMapLoc() ]=])
        server.player.spaceMove(uid, '比奇县_0', x + 490, y + 360)
    end

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, args)
            dialog.post(uid, questPath, '这位侠客，请一定要帮帮我啊！',
            {
                dialog.link('npc_leave', '假装没听见，我走了！'),
                dialog.link('npc_ask', '什么事啊？'),
            })
        end,

        npc_leave = function(uid, args)
            dialog.post(uid, questPath, '请一定要帮帮我...啊？太无情了！',
            dialog.link(SYS_EXIT, '结束'))

            fnLeaveMap(uid)
        end,

        npc_ask = function(uid, args)
            dialog.post(uid, questPath,
            {
                '呜呜...不久前我的孩子在夜市被商人给抢走了。一定要帮我找回孩子啊！',
                '不知道他把孩子带走到底想干什么...一定要...拜托您了！',
            },
            {
                dialog.link('npc_accept', '知道了，我会去帮你找回孩子的！'),
                dialog.link('npc_refuse', '我没有这闲工夫。'),
            })
        end,

        npc_accept = function(uid, args)
            dialog.post(uid, questPath, '多谢了！听说夜市的商人在(452,297)，请一定要帮我找回孩子啊！',
            dialog.link(SYS_EXIT, '结束'))

            fnLeaveMap(uid)
            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_refuse = function(uid, args)
            dialog.post(uid, questPath, '这样啊...呜呜...天下之大，竟然没有同情失去孩儿母亲心的侠客吗？...',
            dialog.link(SYS_EXIT, '结束'))

            fnLeaveMap(uid)
        end,

        npc_extra = function(uid, args)
            dialog.post(uid, questPath, '这到底是怎么回事？那就是说女人回来了？',
            {
                dialog.link(SYS_ENTER, '问一下这个来路不明的女人！'),
                dialog.link(SYS_EXIT, '还是算了！'),
            })
        end,
    })
]])

uidRemoteCall(getNPCharUID('比奇县_0', '母子石像_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = false,
        [SYS_ENTER] = function(uid, args)
            server.player.spaceMove(uid, '比奇县_0_003', 28, 35)
            server.npc.runHandler(getNPCharUID('比奇县_0_003', '石母_1'), uid, {SYS_EPQST, questName}, 'npc_extra')
        end,
    })

    addTrigger(SYS_ON_APPEAR, function(uid)
        if not isPlayer(uid) then
            return
        end

        dialog.post(uid, questPath, '这石头的样子真奇怪...',
        {
            dialog.link(SYS_ENTER, '过去看看？'),
            dialog.link(SYS_EXIT, '感觉很奇怪，我还是离远点比较好...'),
        })
    end)
]])
