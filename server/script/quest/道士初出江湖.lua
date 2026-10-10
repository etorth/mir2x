-- QuestDiary/NQ_BASE/taoistbase.txt, 初出江湖 for 道士, 战士初出江湖.lua and 法师初出江湖.lua are the other professions
--
-- when done:
--      1. dbAddFlag('done_wang_book')
--
-- legacy as written, not tidied:
--     taoistbase_11 thanks the player for helping 大老板 on the level 6 route, which skips the errands
--     001.txt has no entry for flags 102-105, 131, 163 and 164, the journal texts here are new

setQuestFSMTable(
{
    [SYS_DONE] = function(uid)
        setQuestDesp{uid=uid, '把古籍交给了王大人，得到了辛苦费和青铜头盔。'}
    end,

    -- [102]
    [SYS_ENTER] = function(uid, value)
        setQuestDesp{uid=uid, '按照士官的嘱托，去找大老板（394，169）。'}

        -- @Dosa_GO_YONG, while the errands run
        setupNPCQuestBehavior('道馆_1', '士官_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(394,169)', {close = true, args = [=[{'道馆_1',394,169}]=], prefix = '和杂货店', suffix = '的大老板道友聊过了吗？'}),
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_fly_to_loc = function(uid, value)
                    uidRemoteCall(uid, value,
                    [=[
                        local dstStr = ...
                        spaceMove(load('return' .. dstStr)())
                    ]=])
                end,
            }
        ]])

        -- @Dosa_DQ_START
        setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '是士官派你来的？',
                        '嗯，那么先吩咐你做件简单的事儿吧！你能去把这个护身符交给武器库的<t color="red">阿潘</t>道友吗？',
                    },
                    dialog.link('npc_accept', '好的，没问题。'))
                end,

                -- @Dosa_DQ_START_2
                npc_accept = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from=SYS_ENTER, state='quest_deliver_parcel'}) then
                        return
                    end

                    server.player.addItem(uid, '道力护身符', 1)
                    dialog.post(uid, questPath,
                    {
                        '阿潘道友还在等着呢！尽快把这个护身符给他带过去吧！',
                        dialog.link('npc_fly_to_loc', '(429,120)', {close = true, args = [=[{'道馆_1',429,120}]=], prefix = '从这出去再向右上方一直走就是阿潘道友所在的武器库入口。准确位置在', suffix = '。'}),
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_fly_to_loc = function(uid, value)
                    uidRemoteCall(uid, value,
                    [=[
                        local dstStr = ...
                        spaceMove(load('return' .. dstStr)())
                    ]=])
                end,
            }
        ]])
    end,

    -- [103]
    quest_deliver_parcel = function(uid, value)
        setQuestDesp{uid=uid, '把道力护身符交给阿潘（429，120）。'}

        -- @Dosa_GO_JUNG
        setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(429,120)', {close = true, args = [=[{'道馆_1',429,120}]=], prefix = '出去后向右上方一直走就是武器库的入口。位置在', suffix = '。见到阿潘道友后把道力护身符交给他。'}),
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_fly_to_loc = function(uid, value)
                    uidRemoteCall(uid, value,
                    [=[
                        local dstStr = ...
                        spaceMove(load('return' .. dstStr)())
                    ]=])
                end,
            }
        ]])

        -- @Dosa_TAKE_BUJOUK
        setupNPCQuestBehavior('武器仓库_1_001', '阿潘_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    if not server.player.hasItem(uid, '道力护身符', 1) then
                        dialog.post(uid, questPath,
                        {
                            '道力护身符在哪啊？',
                            '是不是没带啊！可不要和我开玩笑哦！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_deliver_parcel', state='quest_request_material'}) then
                        return
                    end

                    server.player.removeItem(uid, '道力护身符', 1)
                    server.player.addItem(uid, '匕首', 1)
                    dialog.post(uid, questPath,
                    {
                        '嗯，这就是以前我要的护身符啊！要是你不送来的话我就要去催大老板道友了，做得不错啊！',
                        '送你一把我们店里卖的匕首就当是报答你了，希望能好好使用它哦！',
                        '现在再去找找大老板道友吧，或许又有什么事情要派施主去做呢！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [163]
    quest_request_material = function(uid, value)
        setQuestDesp{uid=uid, '送完东西了，回去找大老板（394，169）。'}

        -- @Dosa_RETURN_YONG
        setupNPCQuestBehavior('武器仓库_1_001', '阿潘_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(394,169)', {close = true, args = [=[{'道馆_1',394,169}]=], prefix = '大老板道友呆的杂货店在', suffix = '那儿。快回去看看吧！'}),
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_fly_to_loc = function(uid, value)
                    uidRemoteCall(uid, value,
                    [=[
                        local dstStr = ...
                        spaceMove(load('return' .. dstStr)())
                    ]=])
                end,
            }
        ]])

        -- @Dosa_SEARCH_PI, sets [104] as it tells
        setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_request_material', state='quest_collect_material'}) then
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '把护身符交给啊潘道友了吧？' ..
                        '那么我会信任施主并且再拜托施主办另外的事儿的！' ..
                        '倒没什么特别的，只是在道馆北部的灌木林中最近总有怪物出没，跑出来骚扰百姓，所以需要许多护身符。' ..
                        '但是我又有其他的急事要办没时间去弄制护身符所需的鸡血，所以希望你替我收集<t color="green">2</t>瓶<t color="red">鸡血</t>来！',
                        '嗯，只要去猎到鸡自然就会有鸡血了，所以不用特别担心！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [104]
    quest_collect_material = function(uid, value)
        setQuestDesp{uid=uid, '给大老板带去2瓶鸡血。'}

        -- legacy 阿潘 talks about the shop at [163] only
        clearNPCQuestBehavior('武器仓库_1_001', '阿潘_1', uid)

        -- @Dosa_MQ_COMPLETE
        setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, '哦！是来找鸡血吧！',
                    dialog.link('npc_hand_in', '是的，来找鸡血。'))
                end,

                -- @Dosa_MQ_COMPLETE_4
                npc_hand_in = function(uid, value)
                    if not server.player.hasItem(uid, '鸡血', 2) then
                        dialog.post(uid, questPath,
                        {
                            '去猎几只鸡，鸡血自然就会有了！',
                            '嗯，只要弄来<t color="red">两瓶鸡血</t>就可以了！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_collect_material', state='quest_prepare_to_wang'}) then
                        return
                    end

                    server.player.removeItem(uid, '鸡血', 2)
                    server.player.addItem(uid, SYS_GOLDNAME, 1000)
                    dialog.post(uid, questPath,
                    {
                        '辛苦了！',
                        '多亏了你，我才能及时画完所有的护身符啊！',
                        '这是辛苦费请你收下，再回去找找士官吧！',
                        '或许还有别的事情要你做呢！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [105]
    quest_prepare_to_wang = function(uid, value)
        setQuestDesp{uid=uid, '完成了大老板的委托，修炼到6级后找士官领取新的任务。'}

        -- 07Grocery_DoGwan-1_007 NPC_Main_0_5
        setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, '你找过<t color="red">士官</t>了吗？',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Dosa_CQ_START1
        setupNPCQuestBehavior('道馆_1', '士官_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, string.format('<t color="red">%s</t>施主的实力真是大有所增啊！', server.player.getName(uid)),
                    dialog.link('npc_ask_more', '没有什么别的我能做的事儿了吧！'))
                end,

                -- @Dosa_CQ_START1_3
                npc_ask_more = function(uid, value)
                    if server.player.getLevel(uid) < 6 then
                        dialog.post(uid, questPath,
                        {
                            '下次交给你做的事可能有点难，等你修炼一段时间后再来吧！',
                            '修炼到了6级再来吧！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    -- @Dosa_CQ_START1_4
                    dialog.post(uid, questPath,
                    {
                        string.format('<t color="red">%s</t>施主的实力真是大有所增啊！', server.player.getName(uid)),
                        '现在你需要摆脱道馆的周围，将视野放到更宽广的地方去才行。',
                        '从这里通过东南方的通路<t color="red">(516，580)</t>到达比奇县后就可以到达首都比奇省。那个地方是政治、经济、文化的中心地。想修炼成为道士，一定要了解人间苦暖才行，所以这是个必须去的地方！',
                        '正好贫道有一样东西要送到比奇省，这件事情就拜托给你吧！',
                    },
                    dialog.link('npc_accept_book', '明白了。'))
                end,

                -- @Dosa_CQ_START1_5
                npc_accept_book = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_prepare_to_wang', state='quest_deliver_book'}) then
                        return
                    end

                    server.player.addItem(uid, '古籍', 1)
                    server.player.addItem(uid, '治愈术', 1)
                    dialog.post(uid, questPath,
                    {
                        '往比奇省西南方走，<t color="red">就能找到王大人</t>了。详细的位置在比奇县<t color="red">(389，396)</t>。',
                        '找到他，然后把这本书转交给他，他自然会支付给你报酬。',
                        '通过比奇省的东南部通路<t color="red">(516，580)</t>到达比奇县后，就可以找到比奇省了。',
                        '对了，别忘了把这本武功秘笈给道士高手<t color="red">清明子</t>。',
                        string.format('这位高手能给像<t color="red">%s</t>施主这样的道士入门者传授一些基本的魔法，施主一定会有所收获的。', server.player.getName(uid)),
                        '清明子就在本馆内。从本馆左边往上走就可以找到了。准确位置在<t color="red">(429，96)</t>。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [131]
    quest_deliver_book = function(uid, value)
        setQuestDesp{uid=uid, '把古籍交给比奇县王大人（389，396），并去拜访清明子（429，96）。'}

        -- reset [102] 28 takes 大老板 back to plain trade
        clearNPCQuestBehavior('仓库_1_007', '大老板_1', uid)

        -- @Dosa_GO_WANG
        setupNPCQuestBehavior('道馆_1', '士官_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '去见完比奇省的王大人，还要请您去拜访本馆的清明子！',
                        '王大人在比奇省的西南部就可以找到。准确位置是比奇县<t color="red">(389，396)</t>。',
                        '通过比奇省的东南通路<t color="red">(516，580)</t>到达比奇县后，便可以找到比奇省了。',
                        '清明子就在本馆内。从本馆左边往上走就可以找到了。准确位置在<t color="red">(429，96)</t>。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Dosa_CQ_COMPLETE
        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, '来此有何贵干啊？',
                    dialog.link('npc_hand_in', '请问阁下是王大人吗？'))
                end,

                -- @Dosa_CQ_COMPLETE_0
                npc_hand_in = function(uid, value)
                    if not server.player.hasItem(uid, '古籍', 1) then
                        dialog.post(uid, questPath, '难道你是受道馆的士官之托而来的人？',
                        {
                            dialog.link('npc_no_book', '嗯，正是在下'),
                            dialog.link('npc_deny', '您认错人了吧！'),
                        })
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_deliver_book', state=SYS_DONE}) then
                        return
                    end

                    server.player.removeItem(uid, '古籍', 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 1000)
                    server.player.addItem(uid, '青铜头盔', 1)
                    server.player.dbAddFlag(uid, 'done_wang_book')
                    dialog.post(uid, questPath,
                    {
                        '我就是你要找的王某人<t wrap="0">···</t>咳嗯<t wrap="0">···</t>',
                        '哦？带来了道馆的士官送给我的东西吗？啊哈，这就是我以前想要的古书。远道而来，辛苦你啦！',
                        '这是给你的辛苦费，请收下吧！',
                        '对了，或许以后还需要你的帮助呢<t wrap="0">···</t>下次再来吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Dosa_CQ_COMPLETE_2
                npc_no_book = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '可是道馆的士官让你转交给我的书呢？',
                        '可能不知道丢哪儿去了吧<t wrap="0">···</t>',
                        '找到了的话带来吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Dosa_CQ_COMPLETE_3
                npc_deny = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '呵呵<t wrap="0">···</t>出去有一会儿了吧！',
                        '不知是不是在哪儿遇到了什么麻烦<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,
})

-- @Dosa_MQ_START
uidRemoteCall(getNPCharUID('道馆_1', '士官_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '初出江湖',

        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '道士') then
                return false
            end
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        -- @Dosa_MQ_START_0: the book at once from level 6, a wait from level 4, the errands below it
        [SYS_ENTER] = function(uid, value)
            local level = server.player.getLevel(uid)
            if level >= 4 and level < 6 then
                -- @Dosa_MQ_START_1
                local name = server.player.getName(uid)
                dialog.post(uid, questPath,
                {
                    string.format('现在没有什么合适的任务交给<t color="red">%s</t>施主做呀！', name),
                    '请级别高一点，修练到6级以上再来吧！',
                    string.format('祝<t color="red">%s</t>好运噢！', name),
                },
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            -- @Dosa_MQ_START_2 and @Dosa_MQ_START_6 say the same
            dialog.post(uid, questPath,
            {
                '好的。首先贫道会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下道馆内的事情！',
                '这之前如果有什么不明白的地方，尽管来问吧！',
            },
            {
                dialog.link('npc_quest', '什么是我要做的事情？'),
                dialog.link('npc_job', '什么是道士？'),
                dialog.link('npc_town', '这道馆是什么样地方？'),
                dialog.link('npc_guide', '士官是做什么事的人？'),
                dialog.link(SYS_EXIT, '退出'),
            })
        end,

        -- @Dosa_MQ_START_3_1 and @Dosa_MQ_START_7_1
        npc_job = function(uid, value)
            dialog.post(uid, questPath,
            {
                '所谓道士就是每天努力洗脱罪过，修身养性，救济人间的人。',
                '我们遵从上仙药手的教诲，追求的是进入一个陌生之地潜心修炼以达到长生不老，得道成仙的目的。',
                '另外，我们还会帮助与怪物战斗的武士，道士的治愈术和防御术对在与怪物战斗中的武士是非常有用的。',
                '主动直接与敌人交手违背了我们上仙药手的教诲。因此在战斗中我们主要采取防御保护的方式。',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        -- @Dosa_MQ_START_3_2 and @Dosa_MQ_START_7_2
        npc_town = function(uid, value)
            dialog.post(uid, questPath,
            {
                '这道馆在很久以前建成至今，有无数的道士都已经修道祭天了！',
                '虽然我们无法得知曾经有多少得道成仙的道人，但是现任馆主波观昊道长的道力却是非常高深莫测的！',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        -- @Dosa_MQ_START_3_3 and @Dosa_MQ_START_7_3
        npc_guide = function(uid, value)
            dialog.post(uid, questPath,
            {
                '贫道在这里负责为本派门生传道！',
                '施主既然了解本派的门道，就不必太费神啦！',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        npc_quest = function(uid, value)
            if server.player.getLevel(uid) >= 6 then
                -- @Dosa_MQ_START_8
                dialog.post(uid, questPath,
                {
                    string.format('<t color="red">%s</t>施主已经帮助了大老板道友，不需要积累经验武功就很高了<t wrap="0">···</t>', server.player.getName(uid)),
                    '啊，这样啊！',
                    '贫道正好要拜托你一件事，要去试试吗？',
                },
                {
                    dialog.link('npc_explain', '嗯，去。'),
                    dialog.link(SYS_EXIT, '请让我考虑一下。'),
                })
            else
                -- @Dosa_MQ_START_4
                dialog.post(uid, questPath,
                {
                    '这个，嗯<t wrap="0">···</t>详细的情况请到收罗杂货的<t color="red">大老板道友</t>那儿打听吧。',
                    dialog.link('npc_fly_to_loc', '(394,169)', {close = true, args = [=[{'道馆_1',394,169}]=], prefix = '大老板道友就在道馆内。从这往下走，在右侧可以看到杂货店，进去就可以见到他了。杂货店入口的大概位置在', suffix = '，请参考一下吧！'}),
                },
                {
                    dialog.link('npc_accept_errand', '好的！', {close = true}),
                    dialog.link(SYS_EXIT, '还是算了。'),
                })
            end
        end,

        -- @Dosa_MQ_START_9
        npc_explain = function(uid, value)
            dialog.post(uid, questPath,
            {
                string.format('<t color="red">%s</t>施主现在要把视野放到更宽广的地方去。', server.player.getName(uid)),
                '首先从这里通过东南方的通路<t color="red">(516，580)</t>到达比奇县后就可以找到首都比奇省。那个地方是政治、经济、文化的中心地。想修炼成为道士，一定要了解人间苦暖才行，所以这是个必须去的地方！',
                '正好贫道有一样东西要送到比奇省，这件事情就拜托给你吧！',
            },
            dialog.link('npc_accept_book', '明白了。'))
        end,

        npc_fly_to_loc = function(uid, value)
            uidRemoteCall(uid, value,
            [=[
                local dstStr = ...
                spaceMove(load('return' .. dstStr)())
            ]=])
        end,

        npc_accept_errand = function(uid, value)
            server.quest.setState(questUID, {uid=uid, from=SYS_LUANIL, state=SYS_ENTER})
        end,

        -- @Dosa_MQ_START_10
        npc_accept_book = function(uid, value)
            if not server.quest.setState(questUID, {uid=uid, from=SYS_LUANIL, state='quest_deliver_book'}) then
                return
            end

            server.player.addItem(uid, '古籍', 1)
            server.player.addItem(uid, '治愈术', 1)
            dialog.post(uid, questPath,
            {
                '往比奇省西南方向走，就能找到叫做<t color="red">王大人</t>的人。准确位置在比奇县<t color="red">(389，396)</t>。',
                '找到他后把这本书交给他，报酬他自会支付给你的。',
                '通过比奇省的东南部通路<t color="red">(516，580)</t>到达比奇县后，就可以找到比奇省了。',
                '对了，千万别忘了带上贫道的武功秘笈给道士高手<t color="red">清明子</t>看啊！',
                string.format('这位高手能给像<t color="red">%s</t>施主这样的道士入门者传授一些基本的魔法，要是去的话一定能有所收获。', server.player.getName(uid)),
                '清明子就在本馆内。从本馆左边往上走就可以找到他了。准确位置在<t color="red">(429，96)</t>。',
            },
            dialog.link(SYS_EXIT, '结束'))
        end,
    })
]])
