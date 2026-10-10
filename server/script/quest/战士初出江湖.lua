-- QuestDiary/NQ_BASE/warriorbase.txt, 初出江湖 for 战士, 道士初出江湖.lua and 法师初出江湖.lua are the other professions
--
-- when done:
--      1. dbAddFlag('done_wang_book')
--
-- legacy as written, not tidied:
--     warriorbase_32 sends the player to the 铁匠铺 for 肉店金老板
--     warriorbase_26 teaches Alt+click harvesting, mir2x has none, 牛 drops the meat
--     warriorbase_13 thanks the player for helping 肉店金老板 on the level 6 route, which skips the errands
--     001.txt has no entry for flags 102-105, 131, 163 and 164, the journal texts here are new

setQuestFSMTable(
{
    [SYS_DONE] = function(uid)
        setQuestDesp{uid=uid, '把古籍交给了王大人，得到了辛苦费和青铜头盔。'}
    end,

    -- [102]
    [SYS_ENTER] = function(uid, value)
        setQuestDesp{uid=uid, '按照上官小姐的嘱托，去找肉店金老板（425，274）。'}

        -- @Junsa_GO_BUCHER, while the errands run
        setupNPCQuestBehavior('边境城市_01', '上官小姐_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(425,274)', {close = true, args = [=[{'边境城市_01',425,274}]=], prefix = '和肉店金老板', suffix = '聊过了吗？'}),
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

        -- @Junsa_DQ_START
        setupNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid,
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
                        '是上官小姐让你来的吗？',
                        '嗯<t wrap="0">···</t>现在最好先从送东西开始做起吧。也没什么特别的，就是把这碗肉汤给铁匠铺的德秀送去就行，本来我是想亲自去的，可是现在手头上有点儿忙<t wrap="0">···</t>',
                    },
                    dialog.link('npc_accept', '嗯，就交给我吧！'))
                end,

                -- @Junsa_DQ_START_2
                npc_accept = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from=SYS_ENTER, state='quest_deliver_parcel'}) then
                        return
                    end

                    server.player.addItem(uid, '肉汤', 1)
                    dialog.post(uid, questPath,
                    {
                        '那么，趁着没凉之前赶快送过去吧！',
                        dialog.link('npc_fly_to_loc', '(459,279)', {close = true, args = [=[{'边境城市_01',459,279}]=], prefix = '他现在一定饿极了<t wrap="0">···</t>对了，德秀的铁匠铺往右走一点就到了。大概在', suffix = '附近！'}),
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
        setQuestDesp{uid=uid, '把肉汤交给德秀（459，279）。'}

        -- @Junsa_GO_DUKSU
        setupNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(459,279)', {close = true, args = [=[{'边境城市_01',459,279}]=], prefix = '汤都快要凉了！一直向右走就是德秀的铁匠铺！可能去', suffix = '那儿可以找到的！'}),
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

        -- @Junsa_TAKE_GOGIGUK
        setupNPCQuestBehavior('边境城市_01', '德秀_1', uid,
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
                    if not server.player.hasItem(uid, '肉汤', 1) then
                        dialog.post(uid, questPath,
                        {
                            '肉汤在哪？',
                            '好像这肉汤都发出臭味儿了，不要耍弄人啊！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_deliver_parcel', state='quest_request_material'}) then
                        return
                    end

                    server.player.removeItem(uid, '肉汤', 1)
                    server.player.addItem(uid, '匕首', 1)
                    dialog.post(uid, questPath,
                    {
                        '呵呵~饿了半天了！多谢你给我带吃的过来啊！',
                        '送你一把我们店里卖的匕首就当是报答你了。挺有用的。',
                        '现在再去铁匠铺找一下肉店金老板，或许有什么让你做的事儿<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [163]
    quest_request_material = function(uid, value)
        setQuestDesp{uid=uid, '送完东西了，回去找肉店金老板（425，274）。'}

        -- @Junsa_RETURN_KIM
        setupNPCQuestBehavior('边境城市_01', '德秀_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(425,274)', {close = true, args = [=[{'边境城市_01',425,274}]=], prefix = '肉铺店不是在', suffix = '那儿嘛！快点回去看看吧！'}),
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

        -- @Junsa_SEARCH_GOGI
        setupNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid,
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
                        '做的很不错啊！我会信任你并且拜托你做其他更难一点儿的事情！',
                        '我们店有很多的回头客儿，他们的食性不是一般的挑剔，如果不是最好质量的肉他们连看都不看一眼！',
                        '质量好的肉当然利润也会很高，所以我当然不能回绝这样的生意。但供应量不足却一直是个问题！要是不能提供足量高质量的肉的话，没准儿什么时候顾客就会被别的肉铺店给抢过去！所以拜托你帮我找些<t color="red">质量在10以上的牛肉</t>来。',
                        '嗯，想要听听采集肉的注意事项吗？',
                    },
                    {
                        dialog.link('npc_tell_how', '请告诉我吧！'),
                        dialog.link('npc_known', '已经知道了！'),
                    })
                end,

                -- @Junsa_SEARCH_GOGI_2_1
                npc_tell_how = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_request_material', state='quest_collect_material'}) then
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '可以得到肉的动物有鸡和鹿，还有虽然不在这附近栖息的羊。打到猎物后按住Alt键，然后点击鼠标左键就可以把肉放到包里了。',
                        '好吧，现在就出去寻找牛，然后把<t color="red">质量在10以上的牛肉</t>拿来！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Junsa_SEARCH_GOGI_2_2
                npc_known = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_request_material', state='quest_collect_material'}) then
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '知道的话那就太好了！',
                        '我在这等你，<t color="red">质量10以上的牛肉</t>就拜托给你了哦！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [104]
    quest_collect_material = function(uid, value)
        setQuestDesp{uid=uid, '给肉店金老板带去质量10以上的牛肉。'}

        -- legacy 德秀 talks about the shop at [163] only
        clearNPCQuestBehavior('边境城市_01', '德秀_1', uid)

        -- @Junsa_MQ_COMPLETE
        setupNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid,
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
                    dialog.post(uid, questPath, '哦，找来上等的牛肉吗？',
                    dialog.link('npc_hand_in', '嗯，找来了！'))
                end,

                -- @Junsa_MQ_COMPLETE_4
                npc_hand_in = function(uid, value)
                    if not server.player.hasItemQuality(uid, '牛肉', 10, 1) then
                        dialog.post(uid, questPath,
                        {
                            '嗯？看来这不是<t color="red">质量在10以上的牛肉</t>啊？',
                            '你好象是看错了啊？',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_collect_material', state='quest_prepare_to_wang'}) then
                        return
                    end

                    -- legacy take 牛肉 1 takes any copy, not the one checkdura looked at
                    server.player.removeItemQuality(uid, '牛肉', 10, 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 1000)
                    dialog.post(uid, questPath,
                    {
                        '辛苦了！',
                        '多亏了你才满足了回头客儿们的要求。',
                        '这是给你的辛苦费！',
                        '现在没什么事了，再去上官小姐<t color="red">(461，257)</t>那儿看看吧！',
                        '或者还有别的事情要你去做呢！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [105]
    quest_prepare_to_wang = function(uid, value)
        setQuestDesp{uid=uid, '完成了肉店金老板的委托，修炼到6级后找上官小姐领取新的任务。'}

        -- 01Meet_Kugkyung-01 NPC_Main_0_5
        setupNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid,
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
                    dialog.post(uid, questPath, '你找过<t color="red">上官小姐</t>了？有什么事？',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Junsa_CQ_START1
        setupNPCQuestBehavior('边境城市_01', '上官小姐_1', uid,
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
                    dialog.post(uid, questPath, '看来在肉铺店交给你的事情都做完了吧！',
                    dialog.link('npc_ask_more', '没有什么别的我能做的事儿了吗？'))
                end,

                -- @Junsa_CQ_START1_3
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

                    -- @Junsa_CQ_START1_4
                    dialog.post(uid, questPath,
                    {
                        string.format('现在<t color="red">%s</t>实力也有大有所增了啊！', server.player.getName(uid)),
                        '所以现在要慢慢脱离边境城市，把视野放到更加宽广的地方去才行！',
                        '从这儿通过村子东北部的通路<t color="red">(563，68)</t>到达比奇县后，就可以找到首都比奇省。那里是政治、经济、文化的中心地。也是想要成为战士一定要去的地方。',
                        '正好我有样东西要送到比奇省去，这件事就拜托给你去办吧！',
                    },
                    dialog.link('npc_accept_book', '明白了'))
                end,

                -- @Junsa_CQ_START1_5
                npc_accept_book = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_prepare_to_wang', state='quest_deliver_book'}) then
                        return
                    end

                    server.player.addItem(uid, '古籍', 1)
                    server.player.addItem(uid, '基本剑术', 1)
                    dialog.post(uid, questPath,
                    {
                        '往比奇省西南方向走就能找到一个叫做<t color="red">王大人</t>的人了。准确位置在比奇县<t color="red">(389，396)</t>。',
                        '到他那儿以后把这本书转交给他，他会给你报酬的。',
                        '从这儿通过村子东北部的通路<t color="red">(563，68)</t>到达比奇县后，就可以找到首都比奇省。',
                        '对了，我还有一本武功密笈要转交给战士高手<t color="red">龙血先生</t>，千万别忘了。',
                        string.format('那位先生会喜欢传授给像<t color="red">%s</t>您这样的战士入门者一些基本武功，只要去了就一定能学到不少东西的！', server.player.getName(uid)),
                        '龙血先生常常会来欣赏我们村庄右边樱花和瀑布交相映衬的景致，准确位置在<t color="red">(456，302)</t>附近。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [131]
    quest_deliver_book = function(uid, value)
        setQuestDesp{uid=uid, '把古籍交给比奇县王大人（389，396），并去拜访龙血先生（456，302）。'}

        -- reset [102] 28 takes 肉店金老板 back to plain trade
        clearNPCQuestBehavior('边境城市_01', '肉店金老板_1', uid)

        -- @Junsa_GO_WANG
        setupNPCQuestBehavior('边境城市_01', '上官小姐_1', uid,
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
                        '去拜访比奇省的王大人，在去之前请先去拜访一下龙血先生。',
                        '在比奇省西南方就可以找到王大人。准确位置在比奇县<t color="red">(389，396)</t>。',
                        '通过村子东北部的通路<t color="red">(563，68)</t>到达比奇县后，就可以找到首都比奇省。比奇省就在比奇县<t color="red">(480，410)</t>附近，本来就是个大城，很容易找到的。',
                        '龙血先生就在我们村子<t color="red">(456，302)</t>附近。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Junsa_CQ_COMPLETE
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
                    dialog.post(uid, questPath, '找我有什么事吗？',
                    dialog.link('npc_hand_in', '请问阁下是王大人吗？'))
                end,

                -- @Junsa_CQ_COMPLETE_0
                npc_hand_in = function(uid, value)
                    if not server.player.hasItem(uid, '古籍', 1) then
                        dialog.post(uid, questPath, '难道你是受边境城市上官小姐之托而来的人？',
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
                        '我就是王某人<t wrap="0">···</t>咳嗯<t wrap="0">···</t>',
                        '哦？带来了上官小姐送的东西吗？啊哈，这就是我以前想要的古书。远道而来，辛苦你啦！',
                        '这是给你的辛苦费，请收下吧！',
                        '对了，或许以后还需要你的帮助呢<t wrap="0">···</t>下次再来吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Junsa_CQ_COMPLETE_2
                npc_no_book = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '可是上官小姐让你转交给我的书呢？',
                        '可能不知道丢哪儿去了吧<t wrap="0">···</t>',
                        '去找到了再带过来吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Junsa_CQ_COMPLETE_3
                npc_deny = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '呵呵<t wrap="0">···</t>出去已经有一会儿了吧！',
                        '担心会不会出什么事儿啊！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,
})

-- @Junsa_MQ_START
uidRemoteCall(getNPCharUID('边境城市_01', '上官小姐_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '初出江湖',

        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '战士') then
                return false
            end
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        -- @Junsa_MQ_START_0: the book at once from level 6, a wait from level 4, the errands below it
        [SYS_ENTER] = function(uid, value)
            local level = server.player.getLevel(uid)
            if level >= 6 then
                -- @Junsa_MQ_START_6
                dialog.post(uid, questPath,
                {
                    '那么首先呢，我会教你做一些简单容易做的事情，想要试试吗？',
                    '啊，这之前如果有什么不明白的地方，尽管来问吧！',
                },
                {
                    dialog.link('npc_quest', '我要做的事情是什么？'),
                    dialog.link('npc_job', '战士是什么？'),
                    dialog.link('npc_town', '边境城市是个什么样的地方？'),
                    dialog.link('npc_guide', '上官小姐是做什么的人？'),
                    dialog.link(SYS_EXIT, '退出'),
                })

            elseif level >= 4 then
                -- @Junsa_MQ_START_1
                local name = server.player.getName(uid)
                dialog.post(uid, questPath,
                {
                    string.format('现在没有什么合适的任务交给<t color="red">%s</t>您去做！', name),
                    '等级别再高一点，达到了6级再来吧。',
                    string.format('那么祝<t color="red">%s</t>好运哦！', name),
                },
                dialog.link(SYS_EXIT, '结束'))

            else
                -- @Junsa_MQ_START_2
                dialog.post(uid, questPath,
                {
                    '那么，首先我会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下这个边境城市。',
                    '啊，这之前如果有什么不明白的地方，尽管来问吧！',
                },
                {
                    dialog.link('npc_quest', '我要做的事情是什么？'),
                    dialog.link('npc_job', '战士是什么？'),
                    dialog.link('npc_town', '边境城市是个什么样的地方？'),
                    dialog.link('npc_guide', '上官小姐是做什么的人？'),
                    dialog.link(SYS_EXIT, '退出'),
                })
            end
        end,

        -- @Junsa_MQ_START_3_1 and @Junsa_MQ_START_7_1, gender man
        npc_job = function(uid, value)
            if server.player.getGender(uid) then
                dialog.post(uid, questPath,
                {
                    '所谓战士就是依靠体力打仗的人。',
                    '虽然刚开始的时候主要是靠力气，但是进入更高的境界后掌握了内功，就会成为能够灵活自如的运用剑气的武士。',
                    '（不过作为男人还是要有力气才行！不是吗？呵呵呵<t wrap="0">···</t>）',
                },
                dialog.link(SYS_ENTER, '返回'))
            else
                dialog.post(uid, questPath,
                {
                    '所谓战士就是依靠体力打仗的人。',
                    '虽然刚开始的时候主要是靠力气，但是进入更高的境界后掌握了内功，就会成为能够灵活自如的运用剑气的武士。',
                    '（也就是说，如果是女子的话也不是不行的。）',
                },
                dialog.link(SYS_ENTER, '返回'))
            end
        end,

        -- @Junsa_MQ_START_3_2 and @Junsa_MQ_START_7_2
        npc_town = function(uid, value)
            dialog.post(uid, questPath,
            {
                '这个地方在建国初期是曾经是国境守备队驻扎的要塞。',
                '不过以后随着领土的扩张，作为要塞的意义已经慢慢褪色了，现在跟一般的村庄已经没有什么不同的了。',
                '但由于原来是军队驻扎基地，所以军事文化留下了很浓的痕迹，成为选择了战士之路的人们聚集的场所。',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        -- @Junsa_MQ_START_3_3 and @Junsa_MQ_START_7_3
        npc_guide = function(uid, value)
            dialog.post(uid, questPath,
            {
                '强悍的战士很多的话就可以与怪物们作战，这对国家是非常有利的。',
                '我已经和政府签了协议，帮助在这里的战士志愿者成长为优秀的武士。',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        npc_quest = function(uid, value)
            if server.player.getLevel(uid) >= 6 then
                -- @Junsa_MQ_START_8
                dialog.post(uid, questPath,
                {
                    string.format('<t color="red">%s</t>已经帮助了肉店金老板，不需要积累经验，武功就很高啦<t wrap="0">···</t>', server.player.getName(uid)),
                    '嗯<t wrap="0">···</t>有什么适合你的任务呢？',
                    '啊，对了！',
                    '我正好有一事相求，想要试试吗？',
                },
                {
                    dialog.link('npc_explain', '嗯，好的'),
                    dialog.link(SYS_EXIT, '给我点时间考虑一下吧！'),
                })
            else
                -- @Junsa_MQ_START_4
                dialog.post(uid, questPath,
                {
                    '这个，嗯<t wrap="0">···</t>请去肉铺店打听一下具体的情况吧！',
                    dialog.link('npc_fly_to_loc', '(425,274)', {close = true, args = [=[{'边境城市_01',425,274}]=], prefix = '从这往左向下走就可以找到肉铺店了。肉铺店主人是个叫<t color="red">肉店金老板</t>的大叔，现在可能在', suffix = '附近，去和他聊聊吧！'}),
                },
                {
                    dialog.link('npc_accept_errand', '好的！', {close = true}),
                    dialog.link(SYS_EXIT, '还是算了。'),
                })
            end
        end,

        -- @Junsa_MQ_START_9
        npc_explain = function(uid, value)
            dialog.post(uid, questPath,
            {
                string.format('现在对于<t color="red">%s</t>来说，有必要把视野放到加更宽广的地方去才行。', server.player.getName(uid)),
                '从这儿通过村子东北部的通路<t color="red">(563，68)</t>到达比奇县后，就可以找到首都比奇省。那里是政治、经济、文化的中心地。也是想要成为战士一定要去的地方。',
                '正好我有样东西要送到比奇省去，这件事就拜托给你去办吧！',
            },
            dialog.link('npc_accept_book', '明白了'))
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

        -- @Junsa_MQ_START_10
        npc_accept_book = function(uid, value)
            if not server.quest.setState(questUID, {uid=uid, from=SYS_LUANIL, state='quest_deliver_book'}) then
                return
            end

            server.player.addItem(uid, '古籍', 1)
            server.player.addItem(uid, '基本剑术', 1)
            dialog.post(uid, questPath,
            {
                '往比奇省西南方向走就能找到一个叫做<t color="red">王大人</t>的人了。准确位置在比奇县<t color="red">(389，396)</t>。',
                '到他那儿以后把这本书转交给他，他会给你报酬的。',
                '通过村子东北部的通路<t color="red">(563，68)</t>到达比奇县后，就可以找到首都比奇省了。',
                '对了，我还有一本武功密笈要转交给战士高手<t color="red">龙血先生</t>，千万别忘了。',
                string.format('那位先生会喜欢传授给像<t color="red">%s</t>您这样的战士入门者一些基本武功，只要去了就一定能学到不少东西的！', server.player.getName(uid)),
                '龙血先生常常会来欣赏我们村庄右边樱花和瀑布交相映衬的景致，准确位置在<t color="red">(456，302)</t>附近。',
            },
            dialog.link(SYS_EXIT, '结束'))
        end,
    })
]])
