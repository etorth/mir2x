-- QuestDiary/NQ_BASE/wizardbase.txt, 初出江湖 for 法师, 道士初出江湖.lua and 战士初出江湖.lua are the other professions
--
-- when done:
--      1. dbAddFlag('done_wang_book')
--
-- legacy as written, not tidied:
--     wizardbase_12/17 send the player through 东南方的通路(29:452), wizardbase_13/18/19 through 西南部的通路(29:492)
--     wizardbase_30 sends the player to a 金老板 at the 铁匠铺 for 许氏, and wizardbase_31 calls 许氏's (228:194) a 铁匠铺
--     wizardbase_24 teaches Alt+click harvesting, mir2x has none, 鸡 drops the meat
--     wizardbase_11 thanks the player for helping 许氏 on the level 6 route, which skips the errands
--     001.txt has no entry for flags 102-105, 131, 163 and 164, the journal texts here are new

setQuestFSMTable(
{
    [SYS_DONE] = function(uid)
        setQuestDesp{uid=uid, '把古籍交给了王大人，得到了辛苦费和青铜头盔。'}
    end,

    -- [102]
    [SYS_ENTER] = function(uid, value)
        setQuestDesp{uid=uid, '按照南宫小姐的嘱托，去找许氏（228，194）。'}

        -- @Sulsa_GO_BUCHER, while the errands run
        setupNPCQuestBehavior('银杏山谷_02', '南宫小姐_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(228,194)', {close = true, args = [=[{'银杏山谷_02',228,194}]=], prefix = '和肉铺店', suffix = '的许氏聊过了吗？'}),
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

        -- @Sulsa_DQ_START
        setupNPCQuestBehavior('银杏山谷_02', '许氏_1', uid,
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
                        '是南宫小姐叫你来的？',
                        '嗯<t wrap="0">···</t>现在就先从送东西开始做起好啦！也不是什么特别的事儿，就是把这个肉汤送到铁匠师傅那儿。本来我想亲自去送，但是有点儿忙<t wrap="0">···</t>',
                    },
                    dialog.link('npc_accept', '嗯，就交给我来做吧！'))
                end,

                -- @Sulsa_DQ_START_2
                npc_accept = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from=SYS_ENTER, state='quest_deliver_parcel'}) then
                        return
                    end

                    server.player.addItem(uid, '肉汤', 1)
                    dialog.post(uid, questPath,
                    {
                        '那么趁肉汤凉之前赶紧送过去吧！那位朋友现在一定非常饿了<t wrap="0">···</t>',
                        dialog.link('npc_fly_to_loc', '(284,197)', {close = true, args = [=[{'银杏山谷_02',284,197}]=], prefix = '对了，铁匠师傅的铁匠铺一直往右走就行，途中要经过南宫小姐的喷泉池，准确位置可能在', suffix = '附近。'}),
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
        setQuestDesp{uid=uid, '把肉汤交给铁匠师傅（284，197）。'}

        -- @Sulsa_GO_KIM
        setupNPCQuestBehavior('银杏山谷_02', '许氏_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(284,197)', {close = true, args = [=[{'银杏山谷_02',284,197}]=], prefix = '一直往右走，快点把肉汤送给在铁匠师傅。去', suffix = '就可以找到铁匠师傅。'}),
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

        -- @Sulsa_TAKE_GOGIGUK
        setupNPCQuestBehavior('银杏山谷_02', '铁匠师傅_1', uid,
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
                        '现在再去铁匠铺找一下金老板，或许有什么让你做的事儿<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [163]
    quest_request_material = function(uid, value)
        setQuestDesp{uid=uid, '送完东西了，回去找许氏（228，194）。'}

        -- @Sulsa_RETURN_HO
        setupNPCQuestBehavior('银杏山谷_02', '铁匠师傅_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_fly_to_loc', '(228,194)', {close = true, args = [=[{'银杏山谷_02',228,194}]=], prefix = '不是在铁匠铺', suffix = '吗？快去看看吧！'}),
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

        -- @Sulsa_SEARCH_GOGI
        setupNPCQuestBehavior('银杏山谷_02', '许氏_1', uid,
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
                        '做的很不错啊！我会信任你并且拜托你做其他的事情的！',
                        '是这样的，村中长老的花甲寿筵马上就要到了，饭店准备宴席需要质量最好的鸡肉，可是要一下子弄那么多鸡肉可不是一件容易的事儿啊！而且肉这东西是不能保存太长时间的，谁知道会有这样的事而把仓库都堆满呢？',
                        '不管怎么样，现在正为了收集上好的鸡肉忙得团团转，所以也希望你能帮我的忙，就是帮我去四处抓鸡，采集<t color="red">质量在4以上的鸡肉</t>给我带过来。',
                        '你想听听是如何捉鸡的吗？',
                    },
                    {
                        dialog.link('npc_tell_how', '请告诉我吧！'),
                        dialog.link('npc_known', '已经知道了！'),
                    })
                end,

                -- @Sulsa_SEARCH_GOGI_2_1
                npc_tell_how = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_request_material', state='quest_collect_material'}) then
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '可以得到肉的动物有鸡和鹿，还有虽然不在这附近栖息的羊。打到猎物后按住Alt键，然后点击鼠标左键就可以把肉放到包里了。',
                        '好了，现在就出去找<t color="red">质量4以上的鸡肉</t>吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Sulsa_SEARCH_GOGI_2_2
                npc_known = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_request_material', state='quest_collect_material'}) then
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '那样的话真是太好了！',
                        '赶紧去吧，太谢谢你了！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [104]
    quest_collect_material = function(uid, value)
        setQuestDesp{uid=uid, '给许氏带去质量4以上的鸡肉。'}

        -- legacy 铁匠师傅 talks about the shop at [163] only
        clearNPCQuestBehavior('银杏山谷_02', '铁匠师傅_1', uid)

        -- @Sulsa_MQ_COMPLETE
        setupNPCQuestBehavior('银杏山谷_02', '许氏_1', uid,
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
                    dialog.post(uid, questPath, '哦，找来鸡肉了？',
                    dialog.link('npc_hand_in', '是的，找来了。'))
                end,

                -- @Sulsa_MQ_COMPLETE_4
                npc_hand_in = function(uid, value)
                    if not server.player.hasItemQuality(uid, '鸡肉', 4, 1) then
                        dialog.post(uid, questPath,
                        {
                            '看起来不是高质量的鸡肉啊！',
                            '再和你讲一遍，去找<t color="red">质量在4以上的鸡肉</t>来！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    if not server.quest.setState(questUID, {uid=uid, from='quest_collect_material', state='quest_prepare_to_wang'}) then
                        return
                    end

                    server.player.removeItemQuality(uid, '鸡肉', 4, 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 1000)
                    dialog.post(uid, questPath,
                    {
                        '辛苦了！',
                        '多亏了你寿筵上才有这么好的鸡肉可用啊！',
                        '这是辛苦费。',
                        '现在这件事而已经做完了，再去南宫小姐<t color="red">(264，201)</t>那儿看看吧。',
                        '或许有别的事情要做呢！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [105]
    quest_prepare_to_wang = function(uid, value)
        setQuestDesp{uid=uid, '完成了许氏的委托，修炼到6级后找南宫小姐领取新的任务。'}

        -- 01Meet_Eunhang-02 NPC_Main_0_5
        setupNPCQuestBehavior('银杏山谷_02', '许氏_1', uid,
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
                    dialog.post(uid, questPath, '找过<t color="red">南宫小姐</t>了吗？有什么事吗？',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Sulsa_CQ_START1
        setupNPCQuestBehavior('银杏山谷_02', '南宫小姐_1', uid,
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
                    dialog.post(uid, questPath, '看来好在肉铺店交给你的事情都做完了吧！',
                    dialog.link('npc_ask_more', '没有什么别的我能做的事儿了吧！'))
                end,

                -- @Sulsa_CQ_START1_3
                npc_ask_more = function(uid, value)
                    if server.player.getLevel(uid) < 6 then
                        dialog.post(uid, questPath,
                        {
                            '下次交给你的任务可能会有点难，再修炼一段时间再来吧。',
                            '等等级修炼到6级后再来吧。',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    -- @Sulsa_CQ_START1_4
                    dialog.post(uid, questPath,
                    {
                        string.format('<t color="red">%s</t>的实力好像又大有所增啊！', server.player.getName(uid)),
                        '从现在开始你要渐渐的脱离银杏山谷，要把视野放到更加宽广的地方去才行！',
                        '首先从这里通过东南方的通路<t color="red">(29，452)</t>到达比奇县后就可以找到首都比奇省。那个地方是政治、经济、文化的中心地。要想成为法神一定要去那儿看看才行。',
                        '正好我有样东西要送到比奇省去，这件事儿就拜托给你去办吧！',
                    },
                    dialog.link('npc_accept_book', '明白了'))
                end,

                -- @Sulsa_CQ_START1_5
                npc_accept_book = function(uid, value)
                    if not server.quest.setState(questUID, {uid=uid, from='quest_prepare_to_wang', state='quest_deliver_book'}) then
                        return
                    end

                    server.player.addItem(uid, '古籍', 1)
                    server.player.addItem(uid, '火球术', 1)
                    dialog.post(uid, questPath,
                    {
                        '往比奇省西南方向走有位叫做<t color="red">王大人</t>的人。准确位置在比奇县<t color="red">(389，396)</t>。',
                        '到他那儿以后把这本书交给他，他自然会给你报酬的。',
                        '从这儿通过村子西南部的通路<t color="red">(29，492)</t>到达比奇县后，就能找到比奇省了。',
                        '对了，我还要给法神高手<t color="red">霹雳尊者</t>带一本武功密笈，别忘了去拜访他啊哦！',
                        string.format('他喜欢传授给像<t color="red">%s</t>您这样的法神入门者一些基本的魔法，所以只要去了的话一定能学到许多有用的东西。', server.player.getName(uid)),
                        '听说霹雳尊者住在村子北部山路左边的一棵大树下面。具体位置可能在<t color="red">(265，145)</t>。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    -- [131]
    quest_deliver_book = function(uid, value)
        setQuestDesp{uid=uid, '把古籍交给比奇县王大人（389，396），并去拜访霹雳尊者（265，145）。'}

        -- reset [102] 28 takes 许氏 back to plain trade
        clearNPCQuestBehavior('银杏山谷_02', '许氏_1', uid)

        -- @Sulsa_GO_WANG
        setupNPCQuestBehavior('银杏山谷_02', '南宫小姐_1', uid,
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
                        '拜访住在比奇省的王大人后，另外还要找一下霹雳尊者。',
                        '在比奇省的西南方就可以找到王大人，准确位置可能在比奇县<t color="red">(389，396)</t>。',
                        '从这儿通过村子西南部的通路<t color="red">(29，492)</t>到达比奇县后，就能找到比奇省了。',
                        '听说霹雳尊者住在村子北部山路左边的一棵大树下面。准确位置可能在<t color="red">(265，145)</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @Sulsa_CQ_COMPLETE
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

                -- @Sulsa_CQ_COMPLETE_0
                npc_hand_in = function(uid, value)
                    if not server.player.hasItem(uid, '古籍', 1) then
                        dialog.post(uid, questPath, '难道你是受银杏山谷南宫小姐之托而来的人？',
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
                        '啊，是这样啊？带来了银杏山谷南宫小姐送给我的东西？啊哈，这可是我以前想要的的古书。远道而来，辛苦你了啦！',
                        '这是辛苦费，请您收下！',
                        '对了，可能还需要你帮我做点儿什么，下次再来吧！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Sulsa_CQ_COMPLETE_2
                npc_no_book = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '那么南宫小姐叫你转交给我的书呢？',
                        '不知道丢哪去了吧<t wrap="0">···</t>',
                        '快去给我找回来！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                -- @Sulsa_CQ_COMPLETE_3
                npc_deny = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '呵呵<t wrap="0">···</t>出去已经有一会儿了！',
                        '可能是在哪儿遇到了什么麻烦<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,
})

-- @Sulsa_MQ_START
uidRemoteCall(getNPCharUID('银杏山谷_02', '南宫小姐_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '初出江湖',

        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '法师') then
                return false
            end
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        -- @Sulsa_MQ_START_0: the book at once from level 6, a wait from level 4, the errands below it
        [SYS_ENTER] = function(uid, value)
            local level = server.player.getLevel(uid)
            if level >= 6 then
                -- @Sulsa_MQ_START_6
                dialog.post(uid, questPath,
                {
                    string.format('嗯，<t color="red">%s</t>。', server.player.getName(uid)),
                    '那么，首先我会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下银杏山谷吧。',
                    '啊，要是有什么不明白的地方尽管问我吧！',
                },
                {
                    dialog.link('npc_quest', '什么是我要做的事情？'),
                    dialog.link('npc_job', '法神是什么？'),
                    dialog.link('npc_town', '银杏山谷是个什么样的地方？'),
                    dialog.link('npc_guide', '南宫小姐是做什么事情的人？'),
                    dialog.link(SYS_EXIT, '退出'),
                })

            elseif level >= 4 then
                -- @Sulsa_MQ_START_1
                local name = server.player.getName(uid)
                dialog.post(uid, questPath,
                {
                    string.format('现在没有什么合适的任务交给<t color="red">%s</t>施主做啊！', name),
                    '等级别高一点，修炼到6级再来找我吧！',
                    string.format('那么祝<t color="red">%s</t>好运哦！', name),
                },
                dialog.link(SYS_EXIT, '结束'))

            else
                -- @Sulsa_MQ_START_2
                dialog.post(uid, questPath,
                {
                    '那么，首先我会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下银杏山谷吧。',
                    '啊，要是有什么不明白的地方尽管问我吧！',
                },
                {
                    dialog.link('npc_quest', '什么是我要做的事情？'),
                    dialog.link('npc_job', '法神是什么？'),
                    dialog.link('npc_town', '银杏山谷是个什么样的地方？'),
                    dialog.link('npc_guide', '南宫小姐是做什么事情的人？'),
                    dialog.link(SYS_EXIT, '退出'),
                })
            end
        end,

        -- @Sulsa_MQ_START_3_1 and @Sulsa_MQ_START_7_1
        npc_job = function(uid, value)
            dialog.post(uid, questPath,
            {
                '法神是可以使用多种强大魔法的人。',
                '不过相反的是，法神身体很虚弱，所以在战斗的时候要一直十分小心才行。',
                '仅靠修炼是不能成为法神的，作为法神一定要拥有法神的资质，主要是从血统上来继承这种资质的。',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        -- @Sulsa_MQ_START_3_2 and @Sulsa_MQ_START_7_2
        npc_town = function(uid, value)
            dialog.post(uid, questPath,
            {
                '拥有法神资质的几个家族集中居住的地方就是银杏山谷的起始地。',
                '此后这里便成了众多法神聚集的名所！',
                '不过大部分都只是法神的入门者！',
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        -- @Sulsa_MQ_START_3_3 and @Sulsa_MQ_START_7_3
        npc_guide = function(uid, value)
            dialog.post(uid, questPath,
            {
                '我也是这村中法神家族的一员！',
                string.format('为了保持为数不多拥有法神血统的法神们之间的同志意识，培养提高法神整体的势力，我在这里帮助像<t color="red">%s</t>您这样的人能够相对容易的进行修炼！', server.player.getName(uid)),
            },
            dialog.link(SYS_ENTER, '返回'))
        end,

        npc_quest = function(uid, value)
            if server.player.getLevel(uid) >= 6 then
                -- @Sulsa_MQ_START_8
                dialog.post(uid, questPath,
                {
                    string.format('哦，<t color="red">%s</t>已经帮助了肉铺店的许氏，不需要再积累经验武功就已经很高了啊~！', server.player.getName(uid)),
                    '让我看看<t wrap="0">···</t>唔<t wrap="0">···</t>有什么好的差事呢<t wrap="0">···</t>啊！',
                    '我正好有一件事要拜托你去做，想试一试吗<t wrap="0">···</t>？',
                },
                {
                    dialog.link('npc_explain', '嗯！是的'),
                    dialog.link(SYS_EXIT, '请给我点时间考虑一下。'),
                })
            else
                -- @Sulsa_MQ_START_4
                dialog.post(uid, questPath,
                {
                    '这个，嗯<t wrap="0">···</t>请去肉铺店那儿听更详细的故事吧！',
                    dialog.link('npc_fly_to_loc', '(228,194)', {close = true, args = [=[{'银杏山谷_02',228,194}]=], prefix = '从这儿向左去就能够找到肉铺店。肉铺店的主人是个叫<t color="red">许氏</t>的人。可能在', suffix = '附近，去那儿和他聊聊吧~！'}),
                },
                {
                    dialog.link('npc_accept_errand', '好的！', {close = true}),
                    dialog.link(SYS_EXIT, '还是算了。'),
                })
            end
        end,

        -- @Sulsa_MQ_START_9
        npc_explain = function(uid, value)
            dialog.post(uid, questPath,
            {
                string.format('现在对于<t color="red">%s</t>来说，有必要把视野放到加更宽广的地方去才行。', server.player.getName(uid)),
                '首先从这里通过东南方的通路<t color="red">(29，452)</t>到达比奇县后就可以找到首都比奇省。那个地方是政治、经济、文化的中心地。想修炼成为法神，这是个必须去的地方！',
                '正好我有一样东西要送到比奇省，这件事情就拜托给你吧！',
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

        -- @Sulsa_MQ_START_10
        npc_accept_book = function(uid, value)
            if not server.quest.setState(questUID, {uid=uid, from=SYS_LUANIL, state='quest_deliver_book'}) then
                return
            end

            server.player.addItem(uid, '古籍', 1)
            server.player.addItem(uid, '火球术', 1)
            dialog.post(uid, questPath,
            {
                '往比奇省西南方向走有位叫做<t color="red">王大人</t>的人。准确位置在比奇县<t color="red">(389，396)</t>。',
                '到他那儿以后把这本书交给他，他自然会给你报酬的。',
                '从这儿通过村子西南部的通路<t color="red">(29，492)</t>到达比奇县后，就能找到比奇省了。',
                '对了，我还要给法神高手<t color="red">霹雳尊者</t>带一本武功密笈，别忘了去拜访他啊哦！',
                string.format('他喜欢传授给像<t color="red">%s</t>您这样的法神入门者一些基本的魔法，所以只要去了的话一定能学到许多有用的东西。', server.player.getName(uid)),
                '听说霹雳尊者住在村子北部山路左边的一棵大树下面。具体位置可能在<t color="red">(265，145)</t>。',
            },
            dialog.link(SYS_EXIT, '结束'))
        end,
    })
]])
