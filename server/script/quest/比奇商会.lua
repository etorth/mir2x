-- requires:
--      1. dbHasFlag('done_wang_book')
-- when done:
--      1. done_wang_coc is original flag 168, awarded with the first 5000 gold.
--      2. done_wang_coc_expansion is flag 169, awarded with the final 25000 gold.
-- Start the optional expansion through 世玉 before claiming the first-stage reward.
--
-- the expansion, legacy wang.txt, keeps these as it is:
--      1. @INVITE_JABSANG sends a refused grocer ([154]) to @INVITE_JABSANG_21, a label that doesn't exist, _19 to _22
--         are missing; here he goes on to @INVITE_JABSANG_23, the talk that follows. [538] set there is never read.
--      2. @INVITE_JABSANG_25 checks 30000 gold but takes 20000.
--      3. 世玉's wang_22, for a pharmacist paid in full ([167]), is wang_21 reworded; how the teeth were paid isn't
--         recorded here, wang_21 serves both, as one line serves 王大人's wang_9 and wang_10.
--      4. 怡美 asks to be helped first: legacy sends [168] without [170] into the 轻型盔甲 quest itself
--         (@Guyonggap_start), here that quest has an entry of its own and she says the opening of kyunggap_5 only.
--         kyunggap_27, her answer while the grocer hasn't joined, shows in legacy only with [236] of 堕落道士.

_G.fsmName_persuade_librarian  = '劝说图书管理人加入比奇商会'
_G.fsmName_persuade_pharmacist = '劝说药剂师加入比奇商会'
_G.fsmName_expand_coc = '比奇商会势力扩张'

-- the quest flags of the expansion stand for the legacy ones:
--      grocery_friend [153], grocery_refused [154], merchant_grocery [155]
--      jeweler_request [156], merchant_jeweler [157], outfitter_request [158], merchant_outfitter [159]

-- 杂货商, Market_Def/07Grocery_Bichon-0.txt: @INVITE_JABSANG and @JOIN_JABSANG
local function setupExpansionGrocer(uid)
    setupNPCQuestBehavior('比奇县_0', '杂货商_1', uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}
        local dialog = require('include.dialog')

        local function getProgress(uid)
            return uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return
                {
                    friend  = hasQuestFlag(playerUID, 'grocery_friend'),
                    refused = hasQuestFlag(playerUID, 'grocery_refused'),
                    joined  = hasQuestFlag(playerUID, 'merchant_grocery'),
                }
            ]=])
        end

        local function addFlag(uid, flag, desp)
            uidRemoteCall(questUID, uid, flag, desp or false,
            [=[
                local playerUID, flag, desp = ...
                addQuestFlag(playerUID, flag)
                if desp then
                    setQuestDesp{uid=playerUID, fsm=fsmName_expand_coc, desp}
                end
            ]=])
        end

        local function join(uid, texts)
            addFlag(uid, 'merchant_grocery', '杂货商人加入商会成功。')
            dialog.post(uid, questPath, texts,
            dialog.link(SYS_EXIT, '结束'))
        end

        -- checkgold, take, set [155], or set [154] short of the gold
        local function pay(uid, checkGold, takeGold, joinTexts, failTexts)
            if server.player.getGold(uid) < checkGold then
                addFlag(uid, 'grocery_refused')
                dialog.post(uid, questPath, failTexts,
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            server.player.removeGold(uid, takeGold)
            join(uid, joinTexts)
        end

        return
        {
            [SYS_ENTER] = function(uid, value)
                local progress = getProgress(uid)

                -- @JOIN_JABSANG
                if progress.joined then
                    dialog.post(uid, questPath, '替我向王大人转达我的意思吧！',
                    dialog.link(SYS_EXIT, '结束'))

                -- @INVITE_JABSANG_23, see the header
                elseif progress.refused then
                    dialog.post(uid, questPath,
                    {
                        '什么事儿？还是来劝我加入比奇商会吗？',
                        '我没有离开传奇商会的打算，你还是赶紧回去吧！',
                    },
                    dialog.link('npc_reconsider', '再好好考虑一下吧！'))

                -- @INVITE_JABSANG_14
                elseif progress.friend then
                    if server.player.hasItem(uid, '烧酒', 1) then
                        server.player.removeItem(uid, '烧酒', 1)
                        dialog.post(uid, questPath, '嗯？这是什么？',
                        dialog.link('npc_drink', '先喝一杯，边喝边说。'))
                    else
                        dialog.post(uid, questPath,
                        {
                            '什么事儿？还是来劝我加入比奇商会吗？',
                            '虽然跟你很投缘<t wrap="0">···</t>但是毕竟还要讲讲道义啊！我不能就这样抛弃这段时间对我的有恩的崔大夫啊！',
                            '我没有离开传奇商会的打算，你还是赶紧回去吧！',
                            '心一急，嗓子就有点干<t wrap="0">···</t>',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                    end

                -- @INVITE_JABSANG_1
                else
                    dialog.post(uid, questPath,
                    {
                        '让我去加入比奇商会？呵呵！人啊！要讲信义才行。',
                        '难道能因为最近王大人的比奇商会发展的好就背叛一直以来帮助我们的崔大夫？你是无论如何都不能用钱收买我的。',
                        '人的信义是比钱更重要的，我最近虽然想摆脱街头小贩的出身，开一家像样儿的店铺很需要钱，但是也不能因为一点钱就出卖了自己的良心啊！',
                    },
                    dialog.link('npc_think', '再好好的想一下吧！'))
                end
            end,

            -- @INVITE_JABSANG_2
            npc_think = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '“钱”不是重要的，“钱”算什么啊！<t wrap="0">···</t>',
                    '把我看成什么了<t wrap="0">···</t>哼？',
                },
                {
                    dialog.link('npc_money', '<t wrap="0">···</t>多少才行呢？'),
                    dialog.link('npc_leave', '既然您如此固执，那我也没办法。'),
                })
            end,

            -- @INVITE_JABSANG_3
            npc_money = function(uid, value)
                dialog.post(uid, questPath, '.........',
                dialog.link('npc_suggest', '嗯<t wrap="0">···</t>听起来也是个不错的建议。'))
            end,

            -- @INVITE_JABSANG_4
            npc_suggest = function(uid, value)
                dialog.post(uid, questPath, '...............',
                dialog.link('npc_offer', '那么告诉我你需要多少钱吧！'))
            end,

            -- @INVITE_JABSANG_5
            npc_offer = function(uid, value)
                dialog.post(uid, questPath, '咳<t wrap="0">···</t>唔! 1万5千钱左右怎么样<t wrap="0">···</t>',
                {
                    dialog.link('npc_pay15000', '好，给你1万5千钱。'),
                    dialog.link('npc_pay12000', '1万2千钱吧！'),
                    dialog.link('npc_refuse', '1万钱就足够了吧！.....'),
                })
            end,

            -- @INVITE_JABSANG_6
            npc_pay15000 = function(uid, value)
                pay(uid, 15000, 15000,
                {
                    '明白了，那么我会加入比奇商会的。',
                    '但是可要先说清楚了，我绝对不是被你的钱所收买的。',
                    '所谓识时务者为俊杰嘛！我只不过是人在江湖身不由己啊！呵呵呵！',
                },
                {
                    '你不是在跟我开玩笑吧。',
                    '钱有那么了不起吗，为了这点钱就出卖信义？',
                    '刚才我不知怎么回事头脑好像有点发晕。',
                    '但是我说多少你就给多少，这种豪爽的性格的确出乎我的意料啊！',
                    '和你这个朋友很投缘啊<t wrap="0">···</t>',
                })
            end,

            -- @INVITE_JABSANG_7
            npc_pay12000 = function(uid, value)
                pay(uid, 12000, 12000,
                {
                    '我想通了<t wrap="0">···</t>，嗨！没法子啊 ！那么我会加入比奇商会的！',
                    '但是可要先说清楚了，我绝对不是被你的钱所收买的。',
                    '所谓识时务者为俊杰嘛！我只不过是人在江湖身不由己啊！呵呵呵！',
                },
                {
                    '你不是在跟我开玩笑吧？',
                    '钱有那么了不起吗，为了这点钱就出卖信义？',
                    '刚才我不知怎么头脑好像有点发晕。',
                })
            end,

            -- @INVITE_JABSANG_8
            npc_refuse = function(uid, value)
                addFlag(uid, 'grocery_refused')
                dialog.post(uid, questPath,
                {
                    '为了这点钱可不能出卖良心啊！',
                    '你好像很瞧不起人啊!',
                    '不要再和我提起这件事儿了！',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,

            -- @INVITE_JABSANG_9
            npc_leave = function(uid, value)
                dialog.post(uid, questPath, '哦<t wrap="0">···</t>那么<t wrap="0">···</t>你要走？',
                {
                    dialog.link('npc_bargain', '我也没有办法<t wrap="0">···</t>既然你那么讨厌钱。'),
                    dialog.link('npc_friend', '可不是嘛！人的信义是用钱买不到的。'),
                })
            end,

            -- @INVITE_JABSANG_10
            npc_bargain = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '好<t wrap="0">···</t>是个比想象中还要精明的朋友。',
                    '没法子<t wrap="0">···</t>1万2千钱吧！怎么样？',
                },
                {
                    dialog.link('npc_pay10000', '好啊！给你1万钱。'),
                    dialog.link('npc_refuse_7000', '7千钱吧！'),
                })
            end,

            -- @INVITE_JABSANG_11
            npc_pay10000 = function(uid, value)
                pay(uid, 10000, 10000,
                {
                    '明白了，那么我会加入比奇商会的。不过<t wrap="0">···</t>',
                    '但是可要先说清楚了，我绝对不是被你的钱所收买的。',
                    '所谓识时务者为俊杰嘛！我只不过是人在江湖身不由己啊！呵呵呵！',
                },
                {
                    '你不是在跟我开玩笑吧？',
                    '钱有那么了不起吗，为了这点钱就出卖信义？',
                    '刚才我不知怎么头脑好像有点发晕。',
                    '赶紧让开！',
                })
            end,

            -- @INVITE_JABSANG_12
            npc_refuse_7000 = function(uid, value)
                addFlag(uid, 'grocery_refused')
                dialog.post(uid, questPath,
                {
                    '为了这点钱不能出卖良心啊！',
                    '你好像很瞧不起人啊!',
                    '不要再和我提起这件事儿。',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,

            -- @INVITE_JABSANG_13
            npc_friend = function(uid, value)
                addFlag(uid, 'grocery_friend')
                dialog.post(uid, questPath,
                {
                    '咳! 唔... 人的信义是不能用钱收买的。',
                    '但是现在世上很难有向你这样的正人君子啊！',
                    '跟我这样的商人之辈说那样的话<t wrap="0">···</t>呵呵呵。',
                    '和你这个朋友很投缘啊<t wrap="0">···</t>',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,

            -- @INVITE_JABSANG_16
            npc_drink = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '正好嗓子有点干<t wrap="0">···</t>',
                    '谢谢了，咕噜<t wrap="0">···</t>咕噜<t wrap="0">···</t>',
                },
                dialog.link('npc_drink_done', '现在舒服点了吗？'))
            end,

            -- @INVITE_JABSANG_17
            npc_drink_done = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '咕噜<t wrap="0">···</t>嗯<t wrap="0">···</t>不错，好多啦！',
                    '真是意气相通的朋友啊！',
                    '刚刚向要喝酒，就拿着酒来了，呵呵<t wrap="0">···</t>',
                },
                dialog.link('npc_join', '哈哈<t wrap="0">···</t>真是意气相投啊！'))
            end,

            -- @INVITE_JABSANG_18
            npc_join = function(uid, value)
                join(uid,
                {
                    '呵呵呵<t wrap="0">···</t>好就没有笑得这么痛快了！',
                    '你连我这样的杂货商人都没有嫌弃，还这么亲切的对我，我的心也开始摇摆不定了！',
                    '那么我就会加入比奇商会的。',
                })
            end,

            -- @INVITE_JABSANG_24
            npc_reconsider = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '唔<t wrap="0">···</t>真是没办法！',
                    '好吧！你能给我多少钱？',
                },
                dialog.link('npc_pay20000', '我会给你3万钱'))
            end,

            -- @INVITE_JABSANG_25, see the header; short of the gold he stays refused
            npc_pay20000 = function(uid, value)
                pay(uid, 30000, 20000,
                {
                    '两万钱就足够了。那么我现在会加入比奇商会的。',
                    '但是可要先说清楚了，我绝对不是被你的钱所收买的。',
                    '所谓识时务者为俊杰嘛！我只不过是人在江湖身不由己啊！呵呵呵！',
                },
                {
                    '你不是在跟我开玩笑吧？',
                    '钱有那么了不起吗，为了这点钱就出卖信义？',
                    '你好像很瞧不起人啊!',
                    '立刻给我滚开！',
                })
            end,
        }
    ]])
end

-- 恩实, Market_Def/08Accessory_Bichon-0.txt: @GO_WANG_EUNSIL, @INVITE_EUNSIL1, @INVITE_EUNSIL2, @SHOW_BULSA and @JOIN_EUNSIL
local function setupExpansionJeweler(uid)
    setupNPCQuestBehavior('比奇县_0', '恩实_1', uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}
        local dialog = require('include.dialog')

        -- firstReward: 王大人 paid the first reward, [168]
        local function getProgress(uid)
            return uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return
                {
                    grocer      = hasQuestFlag(playerUID, 'merchant_grocery'),
                    requested   = hasQuestFlag(playerUID, 'jeweler_request'),
                    joined      = hasQuestFlag(playerUID, 'merchant_jeweler'),
                    firstReward = dbGetQuestState(playerUID) == 'quest_first_stage_done',
                }
            ]=])
        end

        local function addFlag(uid, flag, desp)
            uidRemoteCall(questUID, uid, flag, desp,
            [=[
                local playerUID, flag, desp = ...
                addQuestFlag(playerUID, flag)
                setQuestDesp{uid=playerUID, fsm=fsmName_expand_coc, desp}
            ]=])
        end

        -- @SHOW_BULSA looks for them in this order, she only looks at it
        local curiosities =
        {
            {'角笛',
            {
                '啊？这个到底是什么呢？',
                '地位很高的半兽人战士在指挥半兽人们时使用的笛子，虽然外表很难看，但是却可以发出非常好听的声音！',
                '真是太美妙了！没想到会看见这种东西<t wrap="0">···</t>',
            }},

            {'不死牌',
            {
                '啊？这个到底是什么呢？',
                '很有古香古色神秘的感觉。',
                '天哪！你说这是从古代流传下来的有着佛师魔法的东西？',
                '真是太美妙了！没想到会看见这种东西<t wrap="0">···</t>',
            }},

            {'灵魂护卫',
            {
                '啊？这个到底是什么呢？',
                '这是能够摄人魂魄的妖怪携带的东西？那么能用这个盛装灵魂？这个实在是太神奇太让我吃惊了！',
            }},

            {'毁灭护身符',
            {
                '啊？这个到底是什么呢？',
                '你说这是能够借助器物的力量突破所设困魔咒的那种叫做不死牌的护身符？现在亲眼见到后，果然能感觉到这股神圣的力量！',
                '真是太美妙了！没想到会看见这种东西<t wrap="0">···</t>',
            }},

            {'沃玛金牌',
            {
                '啊？这个到底是什么呢？',
                '你说什么？连你都不知道这是什么？呵呵呵<t wrap="0">···</t>',
                '不过你说这是在沃玛寺庙找到的东西，所以一定也不会是平常的东西！这里面好像藏着什么大秘密。令我心里七上八下的。',
            }},

            {'地狱神钟',
            {
                '啊？这个到底是什么呢？',
                '过去沃玛教徒们使用过的东西？天哪！这是什么时候的事儿啦<t wrap="0">···</t>',
                '能看到只在传说种听说过的沃玛教遗物我真是太幸运了！',
            }},

            {'灵魂明珠',
            {
                '啊？这个到底是什么呢？',
                '是盛着很久以前牺牲的人们灵魂的玉石？哦！真是听起来让人惋惜的事儿<t wrap="0">···</t>',
                '但是这玉石实在是太漂亮了！我还是头一次见到这种散射出若隐若现光芒的玉石呢<t wrap="0">···</t>',
            }},
        }

        return
        {
            [SYS_ENTER] = function(uid, value)
                local progress = getProgress(uid)

                -- @JOIN_EUNSIL
                if progress.joined then
                    dialog.post(uid, questPath, '那么，请你去转告王大人我会加入比奇商会的！',
                    dialog.link(SYS_EXIT, '结束'))

                -- @SHOW_BULSA
                elseif progress.requested then
                    for _, curiosity in ipairs(curiosities) do
                        if server.player.hasItem(uid, curiosity[1], 1) then
                            dialog.post(uid, questPath, curiosity[2],
                            dialog.link('npc_join', '那么加入比奇商会的事儿<t wrap="0">···</t>'))
                            return
                        end
                    end

                    dialog.post(uid, questPath, '这不是什么新鲜玩艺儿啊！你在耍我吗？',
                    dialog.link(SYS_EXIT, '结束'))

                -- @INVITE_EUNSIL2_1
                elseif progress.grocer then
                    dialog.post(uid, questPath,
                    {
                        '是来说服我加入比奇商会的吧！',
                        '嗯<t wrap="0">···</t>我没有这个打算。',
                    },
                    dialog.link('npc_discuss', '商界的形势已经开始向一边倾斜了。'))

                -- @INVITE_EUNSIL1
                elseif progress.firstReward then
                    dialog.post(uid, questPath, string.format('尽管比奇商会的势力大大增加，但仍不能信任还没能收买<t color="red">杂货商</t>的 <t color="red">%s</t> 啊！', server.player.getName(uid)),
                    dialog.link(SYS_EXIT, '结束'))

                -- @GO_WANG_EUNSIL
                else
                    dialog.post(uid, questPath,
                    {
                        '要我加入比奇商会？',
                        '跟我说这样的话之前，没有什么想先去找王大人跟他说的话吗？',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end
            end,

            -- @INVITE_EUNSIL2_2
            npc_discuss = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '你是说杂货商已经加入到比奇商会了？',
                    '呵呵呵，不要以为我和杂货商是一类人！',
                    '现在这个店铺规模也很大，生意也不错！',
                    '可不是和那种小商贩一样能够随便收买得了的。',
                },
                dialog.link('npc_request', '但是<t wrap="0">···</t>？'))
            end,

            -- @INVITE_EUNSIL2_3
            npc_request = function(uid, value)
                addFlag(uid, 'jeweler_request', '饰品店恩实小姐答应加入商会，但是有条件向你要神奇的东西。')
                dialog.post(uid, questPath,
                {
                    '这个生意一直以来都是靠有不同新花样的新鲜玩艺儿来维持的，可是进来却很难看到新鲜的东西，实在是很郁闷！',
                    '可是又不能扔下店里事情去外面采购些新的货物<t wrap="0">···</t>',
                    '所以你要是能替我找来些一眼就能相中的<t color="red">新鲜玩艺儿</t> 的话，我就会加入比奇商会。',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,

            -- @SHOW_BULSA_3
            npc_join = function(uid, value)
                addFlag(uid, 'merchant_jeweler', '把神奇的东西带给恩实，成功加入商会。')
                dialog.post(uid, questPath,
                {
                    '好的！你帮我搜集到了这么多奇珍异宝，我应该听从你的劝说！',
                    '崔大夫？嗯，管它呢！',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,
        }
    ]])
end

-- 怡美, Market_Def/03Armor_Bichon-0.txt: @GO_WANG_IBBUN, @INVITE_IBBUN, @GIVE_CHOGONG and @JOIN_IBBUN
-- she joins only after 轻型盔甲任务, legacy offers @INVITE_IBBUN with [170] only, see the header
local function setupExpansionOutfitter(uid)
    setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}
        local dialog = require('include.dialog')

        -- firstReward: 王大人 paid the first reward, [168]
        local function getProgress(uid)
            return uidRemoteCall(questUID, uid,
            [=[
                local playerUID = ...
                return
                {
                    grocer      = hasQuestFlag(playerUID, 'merchant_grocery'),
                    requested   = hasQuestFlag(playerUID, 'outfitter_request'),
                    joined      = hasQuestFlag(playerUID, 'merchant_outfitter'),
                    firstReward = dbGetQuestState(playerUID) == 'quest_first_stage_done',
                }
            ]=])
        end

        local function addFlag(uid, flag, desp)
            uidRemoteCall(questUID, uid, flag, desp,
            [=[
                local playerUID, flag, desp = ...
                addQuestFlag(playerUID, flag)
                setQuestDesp{uid=playerUID, fsm=fsmName_expand_coc, desp}
            ]=])
        end

        return
        {
            [SYS_ENTER] = function(uid, value)
                local progress = getProgress(uid)

                -- @JOIN_IBBUN
                if progress.joined then
                    dialog.post(uid, questPath, '如果你去王大人那里的话，就替我转告他我会加入比奇商会的！',
                    dialog.link(SYS_EXIT, '结束'))

                -- @GIVE_CHOGONG
                elseif progress.requested then
                    if server.player.hasItem(uid, '回城卷', 6) then
                        server.player.removeItem(uid, '回城卷', 6)
                        addFlag(uid, 'merchant_outfitter', '把回城卷带给怡美，成功加入比奇商会。')
                        dialog.post(uid, questPath,
                        {
                            '谢谢，现在有了回城卷我就不会像以前那样再遇到那么危险的状况了。',
                            '既然这样我会遵守诺言加入比奇商会的。',
                            '替我转告王大人一声吧！',
                        },
                        dialog.link(SYS_EXIT, '结束'))
                    else
                        dialog.post(uid, questPath, '嗯，如果能给我<t color="red">回城卷 6个</t>，我就会听从你的劝说。',
                        dialog.link(SYS_EXIT, '结束'))
                    end

                -- @GO_WANG_IBBUN
                elseif not progress.firstReward then
                    local name = server.player.getName(uid)
                    if server.player.getLevel(uid) >= 11 then
                        dialog.post(uid, questPath,
                        {
                            string.format('您就是最近一直为比奇商会四处游说的<t color="red">%s</t> 吧！ ！久仰久仰！', name),
                            '看起来你也是来劝说我加入比奇商会的吧！',
                            '但是首先我有点拜托你去办的事儿。加入比奇商会的事儿下次再说吧！',
                            string.format('先去<t color="red">王大人</t>那儿看看怎么样？王大人好像对 <t color="red">%s</t> 您积极的活动非常高兴啊！', name),
                        },
                        dialog.link(SYS_EXIT, '结束'))
                    else
                        dialog.post(uid, questPath,
                        {
                            '让我加我比奇商会吗？',
                            '不过好像跟你这种等级还没有达到11的后生小子没什么可说的。',
                            string.format('不管怎么样还是先去<t color="red">王大人</t>看看如何？王大人好像对 <t color="red">%s</t> 您积极的活动非常高兴啊！', name),
                        },
                        dialog.link(SYS_EXIT, '结束'))
                    end

                -- kyunggap_5 of @Guyonggap_start, see the header
                elseif server.player.getQuestState(uid, '轻型盔甲任务') ~= SYS_DONE then
                    dialog.post(uid, questPath,
                    {
                        '让我加入比奇商会？',
                        '我都要忙死了，你还来跟我说什么话啊！这是比奇省唯一的一家棉布店，你没有看到现在忙的团团转吗？',
                        '要是真的有话和我说就先帮我一把<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '结束'))

                -- kyunggap_27 of @Guyonggap_complete, see the header
                elseif not progress.grocer then
                    dialog.post(uid, questPath,
                    {
                        '又是来劝我加入比奇商会的吧！',
                        string.format('虽说我可以为了 <t color="red">%s</t> 您可以这么做，但是我也是有面子的人啊，连 <t color="red">杂货商</t>都还没有加入比奇商会，我就先加入，这样可不太好吧！', server.player.getName(uid)),
                    },
                    dialog.link(SYS_EXIT, '结束'))

                -- @INVITE_IBBUN_0
                else
                    dialog.post(uid, questPath,
                    {
                        '又是来劝我加入比奇商会的吧<t wrap="0">···</t>',
                        string.format('看来由于 <t color="red">%s</t> 的活动，比奇省的商权现在确实将要全部揽到王大人的手中啊！', server.player.getName(uid)),
                        '如果连我都加入比奇商会的话，崔大夫的传奇商会将彻底瓦解了！',
                    },
                    dialog.link('npc_discuss', '这是大势所趋，你还是作出明智的选择吧！'))
                end
            end,

            -- @INVITE_IBBUN_3
            npc_discuss = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '就像其它商人一样，我也有个条件。',
                    '如果你答应我的条件的话，我就会加入比奇商会。',
                },
                dialog.link('npc_condition', '什么条件呢？'))
            end,

            -- @INVITE_IBBUN_4
            npc_condition = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '像我们这样的普通人由于怪物们的威胁不能在城以外的地方自由活动。',
                    '不久之前我在城外遇到了怪物，使用地牢逃脱卷不但没能逃会城里，反而渐渐到了更加奇怪的地方，差点儿丢了性命。',
                },
                dialog.link('npc_request', '看来你没有回城卷啊！'))
            end,

            -- @INVITE_IBBUN_5
            npc_request = function(uid, value)
                addFlag(uid, 'outfitter_request', '布衣店怡美要求带给她回城卷，可安全移动。')
                dialog.post(uid, questPath,
                {
                    '是啊！我要是有能够使我安全回到城里的回城卷的话我就不会经历那种可怕的事情了。',
                    '所以我的条件就是给我一包回城卷。如果你能找来<t color="red">回城卷 6个</t>给我的话，我就会加入比奇商会。',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,
        }
    ]])
end

setQuestFSMTable(
{
    [SYS_DONE] = function(uid, args)
        local expanded = args == 'expanded' or server.player.dbHasFlag(uid, 'done_wang_coc_expansion')
        setQuestDesp{uid=uid}
        setQuestDesp{uid=uid, expanded and '王大人召集了所有比奇商人，掌握了比奇商界。' or
            '王大人召集有势力的商人加入比奇商会，比奇商业结构会有所改变，由于王大人为你说话，图书管理员很信任你。'}
    end,
    [SYS_ENTER] = function(uid, value)
        uidRemoteCall(getNPCharUID('比奇县_0', '王大人_1'), uid, value,
        [[
            local playerUID, accepted = ...
            local dialog = require('include.dialog')
            if accepted then
                dialog.post(playerUID,
                {
                    '真的太感谢了！我期待着你能带来好消息！',
                    '传奇商会所属的其它商人仍然还有很多，但是现在凭我自己的力量很难一一说服。虽然从好几个方面同时下手。不管怎样？难道不该先避免沦为乞丐吗？所以拜托啦！',
                },
                dialog.link(SYS_EXIT, '好的'))

            else
                dialog.post(playerUID,
                {
                    '不行？',
                    '那么我只好再去找其他人了。',
                },
                dialog.link(SYS_EXIT, '结束'))
            end
        ]])

        if value then
            setQuestState{uid=uid, state='quest_accept_quest'}
        else
            setQuestState{uid=uid, state='quest_refuse_quest'}
        end
    end,

    quest_accept_quest = function(uid, value)
        setQuestState{uid=uid, state='quest_persuade_pharmacist_and_librarian'}
    end,

    quest_refuse_quest = function(uid, value)
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
                    dialog.post(uid, questPath, '那么现在可以帮助我了吗？情况紧急啊！',
                    {
                        dialog.link('npc_accept', '好的', {close = true}),
                        dialog.link('npc_deny', '我没有多余的精力'),
                    })
                end,

                npc_accept = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, from='quest_refuse_quest', state='quest_accept_quest'}
                    ]=])
                end,

                npc_deny = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '唉！',
                        '老天爷啊！真的丢下我不管了吗！？',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    quest_persuade_pharmacist_and_librarian = function(uid, value)
        setQuestDesp{uid=uid, '王大人为了掌握比奇省的商业大权，请你说服图书管理员（473，429）和药剂师（412，410）加入比奇商会。'}
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
                    local fsmState_pharmacist, fsmState_librarian = uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return dbGetQuestState(playerUID, fsmName_persuade_pharmacist),
                               dbGetQuestState(playerUID, fsmName_persuade_librarian)
                    ]=])

                    local donePharmacist = (fsmState_pharmacist == SYS_DONE)
                    local doneLibrarian  = (fsmState_librarian  == SYS_DONE)

                    if (not donePharmacist) and (not doneLibrarian) then
                        dialog.post(uid, questPath,
                        {
                            '我期待着你能带来好消息！',
                            '只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！',
                        },
                        dialog.link(SYS_EXIT, '好的'))

                    elseif not doneLibrarian then
                        dialog.post(uid, questPath,
                        {
                            '还没能拉拢<t color="red">图书管理人</t>啊？再加把劲儿！',
                            '只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！',
                        },
                        dialog.link(SYS_EXIT, '好的'))

                    elseif not donePharmacist then
                        dialog.post(uid, questPath,
                        {
                            '还没能拉拢<t color="red">药剂师</t>啊？再加把劲儿！',
                            '只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！',
                        },
                        dialog.link(SYS_EXIT, '好的'))

                    else
                        dialog.post(uid, questPath,
                        {
                            '噢！我们终于让<t color="red">图书管理人</t>和<t color="red">药剂师</t>从传奇商会退出了！',
                            '他们真的说同意加入比奇商会啦？做得好！做得实在是太棒啦！哈哈哈！',
                        },
                        dialog.link('npc_give_bonus', '你过奖了！'))
                    end
                end,

                -- installed by quest_persuade_pharmacist_and_librarian, which goes on to quest_wait_done at once
                npc_give_bonus = function(uid, value)
                    if not uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        local expanding = dbGetQuestState(playerUID, fsmName_expand_coc) ~= nil
                        return setQuestState{uid=playerUID, from='quest_wait_done', state=expanding and 'quest_first_stage_done' or SYS_DONE}
                    ]=]) then
                        return
                    end

                    -- @WANG_COMPLETE_5
                    dialog.post(uid, questPath, '这个虽然菲薄但是我的诚意，请收下吧！',
                    dialog.link(SYS_EXIT, '结束'))

                    -- a flag of the player, dbAddFlag() is in player.lua, the quest actor has none
                    uidRemoteCall(uid, [=[ dbAddFlag('done_wang_coc') ]=])
                    server.player.addItem(uid, SYS_GOLDNAME, 5000)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
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
                        '早以前这里就是武林人士聚集的地方，呵呵。',
                        '你看起来也像个习武之人，来这里有什么事情吗？',
                    },
                    dialog.link('npc_discuss_1', '我为了劝您加入比奇商会而来此的！'))
                end,

                npc_discuss_1 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '比奇商会？',
                        '啊！啊！知道了！是那个叫做王大人的创办的商人联合会吧！可是我已经加入了崔大夫创办的传奇商会，还是去别的地方试试吧！',
                    },
                    dialog.link('npc_discuss_2', '也就是说无论如何都不行吗？'))
                end,

                npc_discuss_2 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '无论如何？那倒也不是。',
                        '如果你能为我办点事情的话，我也不是不能考虑加入比奇商会的。',
                    },
                    dialog.link('npc_discuss_3', '要我帮你做什么事儿才行呢？'))
                end,

                npc_discuss_3 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '其实最近我正在编撰记录比奇省地理和历史的书籍。如果想要写好这本书的话必然要从各种各样的人那里收集关于比奇省的资料和信息，可是唯独比奇省的卫士们那里不与我合作啊！',
                        '不管怎么样你也是武林人士，可能和他们能够有通融的地方，所以这就是我要拜托你的事情！' ..
                        '值班卫士反正也不能和别人说话，所以希望你能替我去那儿找那些休班卫士从他们那里收集关于比奇省历史的故事。' ..
                        '如果你能做到的话，我会听你的劝告加入比奇商会的。',
                    },
                    {
                        dialog.link('npc_accept', '也许我可以去试试？'),
                        dialog.link(SYS_EXIT, '我和他们也不熟啊！'),
                    })
                end,

                npc_accept = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '哦？你答应我的请求了？',
                        '那太好了，我等着你的好消息！',
                    },
                    dialog.link(SYS_EXIT, '好的！'))

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from=SYS_LUANIL, state=SYS_ENTER}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
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
                    dialog.post(uid, questPath, '我现在特别忙，你有什么事儿吗？',
                    dialog.link('npc_discuss_1', '我来劝说你加入比奇商会！'))
                end,

                npc_discuss_1 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '让我加入王大人的比奇商会？',
                        '你不知道我已经加入传奇商会了吗？呵呵，不过听说王大人那个人也不错，而且比起传奇商会来说条件也要更好。',
                        '但是不管怎么说都要讲点道义啊，怎么能像手心手背那样说翻就翻呢？',
                    },
                    dialog.link('npc_discuss_2', '那就没有别的办法了吗？'))
                end,

                npc_discuss_2 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '嗯！？既然你都这么说了，我倒是有一个建议。',
                        '最近比奇省里流行传染病，配制治疗这种病的药所需的原料毒蛇牙齿非常的紧缺。这种毒蛇牙齿在毒蛇山谷村就有卖的，但是我现在马上要给源源不断而来的病人治病，没有去买药材的时间。',
                        '传奇商会那帮人唯利是图，人命关天的事却无人愿意搭把手帮助我。如果你能够买来足够我们所需的毒蛇牙齿，我就会抛开商人的身份来以医生的角度听从您的劝说。',
                    },
                    {
                        dialog.link('npc_accept', '没问题！'),
                        dialog.link('npc_refuse', '请给我点儿考虑的时间。'),
                    })
                end,

                npc_accept = function(uid, value)
                    dialog.post(uid,
                    {
                        '从这儿向东北部去就能到达毒蛇山谷，可能去(643，15)附近就能够找得到。',
                        '穿过毒蛇山谷一直向东走就会达到那个村庄。在那儿找药商<t color="red">金中医</t>(334，224)向他购买<t color="red">毒蛇牙齿</t>。',
                        '现在患者数量仍然呈增加的趋势，所以还不能推测出以后具体需要多少药材。不管怎么样你都要快去快回。',
                    },
                    dialog.link(SYS_EXIT, '好的'))

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from=SYS_LUANIL, state=SYS_ENTER}
                    ]=])
                end,

                npc_refuse = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '明白吗，年轻人？！',
                        '千万不要太拖延而忘了一切啊！人命关天啊！',
                        '我们所有人啊！',
                    },
                    dialog.link(SYS_EXIT, '退出'))
                end,
            }
        ]])

        setQuestState{uid=uid, state='quest_wait_done'}
    end,

    quest_wait_done = function(uid, value)
        -- if not use this empty state
        -- then fsm stops in quest_persuade_pharmacist_and_librarian, and which setups npc behaviors
        -- but npc behavior is also been set in other fsm, it may overwrite
    end,

    -- the first reward is paid and the expansion goes on, 王大人 pays again when the three merchants joined
    quest_first_stage_done = function(uid)
        setQuestDesp{uid=uid, '王大人召集有势力的商人加入比奇商会，比奇商业结构会有所改变，由于王大人为你说话，图书管理员很信任你。'}
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
                -- @WANG_H_COMPLETE
                [SYS_ENTER] = function(uid, value)
                    local progress = uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return
                        {
                            grocer    = hasQuestFlag(playerUID, 'merchant_grocery'),
                            jeweler   = hasQuestFlag(playerUID, 'merchant_jeweler'),
                            outfitter = hasQuestFlag(playerUID, 'merchant_outfitter'),
                        }
                    ]=])

                    if not progress.grocer then
                        dialog.post(uid, questPath, '嗯<t wrap="0">···</t>现在那个姓崔的家伙的传奇商会就要完蛋啦！呵<t wrap="0">···</t>呵<t wrap="0">···</t>',
                        dialog.link(SYS_EXIT, '结束'))

                    elseif not progress.jeweler then
                        dialog.post(uid, questPath,
                        {
                            '听说<t wrap="0">···</t>你最近<t wrap="0">···</t>',
                            '罢了<t wrap="0">···</t>',
                        },
                        dialog.link(SYS_EXIT, '结束'))

                    elseif not progress.outfitter then
                        dialog.post(uid, questPath,
                        {
                            '听说<t wrap="0">···</t>你最近<t wrap="0">···</t>',
                            '算了<t wrap="0">···</t>',
                        },
                        dialog.link(SYS_EXIT, '结束'))

                    -- @WANG_H_COMPLETE_3
                    elseif server.quest.setState(questUID, {uid=uid, from='quest_first_stage_done', state=SYS_DONE, args='expanded'}) then
                        dialog.post(uid, questPath,
                        {
                            '噢！你来啦！',
                            '真没想到啊！不知不觉中就把传奇商会的商家拉拢到我们这一方啦！',
                            '真是手腕精明啊！由于你的活动终于使我们比奇商会统一了比奇地区商权。',
                            '这是为了报答你的功劳准备的一点小小礼物，请不要谦让务必收下。',
                        },
                        dialog.link(SYS_EXIT, '结束'))

                        server.player.dbAddFlag(uid, 'done_wang_coc_expansion')
                        server.player.addItem(uid, SYS_GOLDNAME, 25000)
                    end
                end,
            }
        ]])
    end,
})

setQuestFSMTable(fsmName_expand_coc,
{
    [SYS_ENTER] = function(uid)
        setQuestDesp{uid=uid, fsm=fsmName_expand_coc, '从卖彩卷的商人那里听到了比奇的商界，快请杂货商人，饰品店恩实小姐，布衣店怡美小姐也加入商会。'}
        setupExpansionGrocer(uid)
        setupExpansionJeweler(uid)
        setupExpansionOutfitter(uid)
        setQuestState{uid=uid, fsm=fsmName_expand_coc, state='quest_expansion_active'}
    end,
    quest_expansion_active = function() end,
})

-- 世玉, Market_Def/09Reinstatement_Bichon-0.txt: the news of 比奇省商界 is how one learns of the expansion
uidRemoteCall(getNPCharUID('比奇县_0', '世玉_1'), getUID(), getQuestName(), fsmName_expand_coc,
[[
    local questUID, questName, expansionFSM = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    -- @main_root_1: 'expansion' from its start till its reward, [152] till [169]
    -- 'news' once the librarian or the pharmacist joined, till the first reward, [165], [166] or [167] without [168]
    local function getNews(uid)
        return uidRemoteCall(questUID, uid,
        [=[
            local playerUID = ...
            if dbGetQuestState(playerUID, fsmName_expand_coc) ~= nil then
                return 'expansion'
            end

            if dbGetQuestState(playerUID) ~= 'quest_wait_done' then
                return nil
            end

            local librarian  = dbGetQuestState(playerUID, fsmName_persuade_librarian ) == SYS_DONE
            local pharmacist = dbGetQuestState(playerUID, fsmName_persuade_pharmacist) == SYS_DONE

            if librarian or pharmacist then
                return 'news', librarian, pharmacist
            end
        ]=])
    end

    setQuestHandler(questName,
    {
        -- @NPC_Main_1 and @NPC_Main_2
        [SYS_LABEL] = function(uid)
            return (getNews(uid) == 'expansion') and '传奇商会' or '比奇省商界'
        end,

        [SYS_CHECKACTIVE] = function(uid)
            return getNews(uid) ~= nil
        end,

        [SYS_ENTER] = function(uid, value)
            local news, librarian, pharmacist = getNews(uid)
            if news == 'news' and librarian and pharmacist then
                -- @BICHUN_SANGGE1_3, set [152]
                if server.quest.setState(questUID, {uid=uid, fsm=expansionFSM, from=SYS_LUANIL, state=SYS_ENTER}) then
                    dialog.post(uid, questPath,
                    {
                        '好像是个从乡下来的人帮助了王大人扩大了比奇商会的势力。',
                        '由于原本拥有绝对优势的崔大夫的传奇商会受到了重大的打击，商权逐渐萎缩，没多久比奇地区的商权就将出现空白的。',
                        '在比奇省中新加入比奇商会的商家除了书店和药剂师之外，还有精肉店、武器商、铁匠铺。',
                        '传奇商会 主要核心成员是<t color="red">布商</t>、<t color="red">首饰店</t>、 <t color="red">杂货商</t>。',
                        '我与其它的商家不同，因为我接受官府的统治保持中立，所以现在是两方势力对等的局面。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                    return
                end
                news = 'expansion'
            end

            -- @BICHUN_SANGGE2
            if news == 'expansion' then
                dialog.post(uid, questPath,
                {
                    '这次由于比奇商会的势力扩张，崔大夫的传奇商会受到重创，现在两个商会的势力呈对等局面。',
                    '如果它的核心成员<t color="red">布商</t>、<t color="red">古董店</t>、 <t color="red">杂货商</t> 也加入到比奇商会的话，崔大夫的传奇商会就名存实亡了。',
                },
                dialog.link(SYS_EXIT, '结束'))

            -- @BICHUN_SANGGE1_1_1
            elseif news == 'news' and librarian then
                dialog.post(uid, questPath,
                {
                    '荣阵阁图书管理人已经加入了比奇商会！',
                    '如果连药剂师也加入比奇商会的话，估计传奇商会其它商人的心也会开始动摇吧！',
                },
                dialog.link(SYS_EXIT, '结束'))

            -- @BICHUN_SANGGE1_2, see the header for wang_22
            elseif news == 'news' then
                dialog.post(uid, questPath,
                {
                    '听说一小会儿之前，药剂师也加入了比奇商会。',
                    '要是连图书管理人也加入比奇商会的话，那么传奇商会里剩下的商人们也都该改变自己的想法了<t wrap="0">···</t>',
                },
                dialog.link(SYS_EXIT, '结束'))
            end
        end,
    })
]])

setQuestFSMTable(fsmName_persuade_librarian,
{
    [SYS_DONE] = function(uid)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '把从所有休班卫士那里听到的比奇历史资料转达给图书管理员，图书管理员承诺加入比奇商会。'}
    end,
    [SYS_ENTER] = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '图书管理员正在搜集史书资料，请和三个休班卫士讲话，再把他们说的故事转达给图书管理员。'}
        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
        [[
            local dialog = require('include.dialog')
            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, '你看起来还没有听到休班卫士的全部故事啊？可以在比奇省内转转就可以找到休班卫士！',
                    dialog.link(SYS_EXIT, '好的'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_1', uid,
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
                        '这鬼天气！真是让人心烦气躁，要是能来口酒润润嗓子该多好啊！',
                        '随便打扰别人真是没礼貌，有什么事情？',
                    },
                    dialog.link('npc_ask_guard_1_info', '你是否知道比奇省的历史？'))
                end,

                npc_ask_guard_1_info = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '嗨！你这个没教养的家伙!求别人办事情至少要应该有点诚意吧？真是不明事理啊！',
                        '唔，嗓子有点干，想去酒店喝杯酒啊！咦？这个月的薪水已经全都喝酒花干净了！钱可真不经花啊！',
                    },
                    dialog.link('npc_guard_1_wait_soju', '退出', {close = true}))
                end,

                npc_guard_1_wait_soju = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from={SYS_ENTER, 'quest_wait_guard_1_and_guard_2_done'}, state='quest_give_guard_1_soju'}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_2', uid,
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
                    if uidRemoteCall(questUID, uid, [=[ return hasQuestFlag(..., 'flag_done_query_guard_2') ]=]) then
                        runEventHandler(uid, questPath, 'npc_guard_2_deny')
                    else
                        runEventHandler(uid, questPath, 'npc_guard_2_accept')
                    end
                end,

                npc_guard_2_accept = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '听说有人找我，是你吗？',
                        '鄙人就是崔某，有什么事儿吗？',
                    },
                    dialog.link('npc_guard_2_give_info', '我想知道有关比奇省的历史。'))
                end,

                npc_guard_2_deny = function(uid, value)
                    dialog.post(uid,
                    {
                        '为什么还要再来？',
                        '我已经把我知道的都告诉你了！',
                    },
                    dialog.link(SYS_EXIT, '退出'))
                end,

                npc_guard_2_give_info = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '说起比奇省的历史<t wrap="0">···</t>',
                        '知道吗？我们的祖先就是讨伐半兽人族地区而派遣出的远征队啊！我们的祖先经过残酷的战斗终于击溃了怪物们。一想到只要再继续坚持战斗一下就可以把怪物们斩草除根，然后可以回到故乡，就都非常高兴。',
                        '可是没想到这时突然发生了始料未及的灾难。这里发生了大地震。原本可以翻过山脉回到家乡的路由于这次大地震导致地壳变动，完全的被隔断了！有的人痛哭流涕，有的人茫然失措。所有人都慌了手脚。',
                        '但是一位优秀的将领重新振作精神，开始在这个地区寻找求生之路。他指挥着他的部下们在赶走半兽人族的地区找到了一片肥沃的土地建立了新的城市。这就是现在的比奇省。',
                        '好了，我已经把知道的基本上全都告诉你啦<t wrap="0">···</t>我也要走啦！',
                    },
                    dialog.link('npc_done_query_guard_2', '谢谢！', {close = true}))
                end,

                npc_done_query_guard_2 = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        -- the flag first, the state it switches to reads it
                        addQuestFlag(playerUID, 'flag_done_query_guard_2')
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from={SYS_ENTER, 'quest_give_guard_1_soju', 'quest_wait_guard_1_and_guard_2_done'}, state='quest_wait_guard_1_and_guard_2_done'}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
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
                    dialog.post(uid, questPath, '是你啊？四处打听比奇省历史的人？',
                    dialog.link('npc_ask_guard_3_info', '是的啊！'))
                end,

                npc_ask_guard_3_info = function(uid, value)
                    dialog.post(uid, questPath, '哦！你也是来问我关于比奇省历史的吗？',
                    dialog.link('npc_guard_3_deny', '是的，请您讲讲比奇省历史的故事吧！'))
                end,

                npc_guard_3_deny = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '喂！我可是卫士中资历最深的！你先去跟其他的人打听之后再来找我吧！',
                        '不能让人小瞧了我<t wrap="0">···</t>',
                    },
                    dialog.link(SYS_EXIT, '退出'))
                end,
            }
        ]])
    end,

    quest_give_guard_1_soju = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '休班卫士嗓子有点干，拿烧酒给他，再打听比奇省的历史。'}
        setupNPCQuestBehavior('比奇县_0', '休班卫士_1', uid,
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
                    if uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 1) ]=]) then
                        dialog.post(uid, questPath,
                        {
                            '啊！又是你，你能不能离我远<t wrap="0">···</t>等等！这是烧酒的味道，好香啊！',
                            string.format('这位%s，能不能给我喝口酒啊！', uidRemoteCall(uid, [=[ return getGender() ]=]) and '少侠' or '姑娘'),
                        },
                        {
                            dialog.link('npc_give_soju', '拿去吧！'),
                            dialog.link('npc_deny_soju', '不愿意。'),
                        })
                    else
                        dialog.post(uid, questPath,
                        {
                            '啊！又是你，你能不能离我远点！？大热天的为什么要三番五次地惹人烦呢？',
                            '要是能有口酒润润嗓子就好啦！',
                        },
                        dialog.link(SYS_EXIT, '退出'))
                    end
                end,

                -- @INVITE_GUARD1_3, the soju first, given back if the switch is refused
                npc_give_soju = function(uid, value)
                    if not server.player.removeItem(uid, '烧酒', 1) then
                        dialog.post(uid, questPath,
                        {
                            '难道已经喝光了吗？',
                            '真扫兴！',
                        },
                        dialog.link(SYS_EXIT, '退出'))
                        return
                    end

                    if not uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        if hasQuestFlag(playerUID, 'flag_done_query_guard_1') then
                            return false
                        end

                        -- the flag first, the state it switches to reads it
                        addQuestFlag(playerUID, 'flag_done_query_guard_1')
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from={'quest_give_guard_1_soju', 'quest_wait_guard_1_and_guard_2_done'}, state='quest_wait_guard_1_and_guard_2_done'}
                    ]=]) then
                        server.player.addItem(uid, '烧酒', 1)
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '啊哈哈哈，真是谢谢你！',
                        '咕噜咕噜，还是烧酒的味道棒啊！咕噜咕噜，炎热的午后能痛快地喝一顿真是神仙般的快活啊！',
                    },
                    dialog.link('npc_ask_info', '不知道你能否告知比奇省的历史？'))
                end,

                npc_deny_soju = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '你是不愿意给我喝呢，还是酒已经被喝光了？',
                        '真扫兴！',
                    },
                    dialog.link(SYS_EXIT, '退出'))
                end,

                npc_ask_info = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '比奇省的历史？嗯？唔<t wrap="0">···</t>',
                        '说起比奇的由来这要追溯到几百年之前啦！' ..
                        '比奇产生之前，西方有几个国家，由于被叫做内日和半兽人的怪物种族袭击一直都处于危险之中，处于威机之中的这几个国家停止了相互之间的战争，协力与怪物们抗争，最后终于赶走了怪物们，但是也全部受到了重创，怪物们的威胁仍然没有完全解除。',
                        '于是这几个国家协力出兵去讨伐怪物们的根据地，那个地方就是比奇地区！',
                        '这些够了吧！',
                    },
                    dialog.link('npc_ask_more_info', '你还知道更多的信息吗？'))
                end,

                npc_ask_more_info = function(uid, value)
                    dialog.post(uid, questPath, '看在烧酒的面子上，我知道的就这些啦！想知道的更多，你也可以去问问其他的卫士。',
                    dialog.link(SYS_EXIT, '好的！'))
                end,
            }
        ]])
    end,

    quest_wait_guard_1_and_guard_2_done = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '请从所有休班卫士那里听到比奇历史的故事，再转达给图书管理员。'}
        local done_guard_1 = hasQuestFlag(uid, 'flag_done_query_guard_1')
        local done_guard_2 = hasQuestFlag(uid, 'flag_done_query_guard_2')

        if not (done_guard_1 and done_guard_2) then
            return
        end

        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
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
                    dialog.post(uid, questPath, '是你啊？四处打听比奇省历史的人？',
                    dialog.link('npc_ask_guard_3_info', '是的啊！'))
                end,

                npc_ask_guard_3_info = function(uid, value)
                    dialog.post(uid, questPath, '哦！你也是来问我关于比奇省历史的吗？',
                    dialog.link('npc_guard_3_answer', '是的，请您讲讲比奇省历史的故事吧！'))
                end,

                npc_guard_3_answer = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '这事儿可就说来话长了<t wrap="0">···</t>',
                        '噢！我可是什么都不知道！呵呵，你还是去问别人吧！',
                    },
                    {
                        dialog.link('npc_give_guard_3_gold', '给他100金币', {close = true, args = '100'}),
                        dialog.link('npc_give_guard_3_gold', '给他1000金币', {close = true, args = '1000'}),
                        dialog.link(SYS_EXIT, '不询问他'),
                    })
                end,

                npc_give_guard_3_gold = function(uid, value)
                    uidRemoteCall(questUID, uid, value, questName,
                    [=[
                        local playerUID, giveGold, questName = ...
                        local nextState = string.format('quest_give_guard_3_%s_gold', giveGold)

                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_wait_guard_1_and_guard_2_done', state=nextState, exitfunc=function()
                            runNPCEventHandler(getNPCharUID('比奇县_0', '休班卫士_3'), playerUID, {SYS_EPUID, questName}, SYS_ENTER)
                        end}
                    ]=])
                end,
            }
        ]])
    end,

    quest_give_guard_3_100_gold = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '再去问第三位休班卫士，听听比奇省的历史。'}
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
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
                    dialog.post(uid, questPath, '嗨哎！？这是干吗？',
                    dialog.link('npc_guard_3_give_info', '请以后买点酒喝什么的吧！'))
                end,

                -- @INVITE_GUARD3_4_1, short of the gold he takes offence, as at 1000
                npc_guard_3_give_info = function(uid, value)
                    if not server.player.removeGold(uid, 100) then
                        uidRemoteCall(questUID, uid, questName,
                        [=[
                            local playerUID, questName = ...
                            setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_give_guard_3_100_gold', state='quest_give_guard_3_1000_gold', exitfunc=function()
                                runNPCEventHandler(getNPCharUID('比奇县_0', '休班卫士_3'), playerUID, {SYS_EPUID, questName}, SYS_ENTER)
                            end}
                        ]=])
                        return
                    end

                    if not uidRemoteCall(questUID, uid,
                    {
                        [=[<par>哦？是嘛，哈哈哈！好吧，我来讲给你听。</par>]=],
                        [=[<par>唔<t wrap="0">···</t>这已经是我所知道的全部故事啦！</par>]=],
                    },
                    [=[
                        local playerUID, texts = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_give_guard_3_100_gold', state='quest_guard_3_give_info', args=texts}
                    ]=]) then
                        server.player.addItem(uid, SYS_GOLDNAME, 100)
                    end
                end,
            }
        ]])
    end,

    quest_give_guard_3_1000_gold = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
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
                        '你<t wrap="0">···</t>你这是做什么？竟敢和保护比奇省治安的我开这种玩笑？',
                        '看来和你是做不了朋友了！要和我比试比试吗？我长这么大还是头一次受到这种污辱！',
                    },
                    dialog.link('npc_guard_3_angry_1', '你千万别误会啊！不是这个意思！'))
                end,

                npc_guard_3_angry_1 = function(uid, value)
                    dialog.post(uid, questPath, string.format('你还狡辩什么啊？你这个%s！', uidRemoteCall(uid, [=[ return getGender() ]=]) and '混小子' or '混丫头'),
                    dialog.link('npc_guard_3_angry_2', '你千万不要误会呀！'))
                end,

                npc_guard_3_angry_2 = function(uid, value)
                    dialog.post(uid, questPath, '哼！呵呵<t wrap="0">···</t>没有别的意思！真的吗？',
                    dialog.link('npc_guard_3_angry_3', '对不起是我错了，请原谅！'))
                end,

                npc_guard_3_angry_3 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '唉！没办法，谁让我年纪大来着呢，原谅一次你吧！这里有1个金币！',
                        '快去买<t color="red">5瓶烧酒</t>来，喝了酒才能消了我的肚子里的火气。别忘了把找还的零钱带回来！',
                    },
                    dialog.link('npc_guard_3_angry_4', '好吧<t wrap="0">···</t>', {close = true}))
                end,

                -- @INVITE_GUARD3_13, give 金币 1
                npc_guard_3_angry_4 = function(uid, value)
                    if uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_give_guard_3_1000_gold', state='quest_give_guard_3_soju'}
                    ]=]) then
                        server.player.addItem(uid, SYS_GOLDNAME, 1)
                    end
                end,
            }
        ]])
    end,

    quest_give_guard_3_soju = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
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
                    local hasSoju    = uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 1) ]=])
                    local hasSoju_x5 = uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 5) ]=])

                    if not hasSoju then
                        dialog.post(uid, questPath, '快去买啊！1个金币还不够吗？',
                        dialog.link(SYS_EXIT, '退出'))

                    elseif not hasSoju_x5 then
                        dialog.post(uid, questPath,
                        {
                            '臭小子，我是说五瓶，快去再买点！',
                            '竟敢不听我的！',
                        },
                        dialog.link(SYS_EXIT, '退出'))

                    else
                        dialog.post(uid, questPath,
                        {
                            '呵！你还真买来了。',
                            '咕噜，咕噜，啊！真是好酒，现在舒服多了。',
                            '对了，你是找我来问什么的来着？',
                        },
                        dialog.link('npc_guard_3_give_info', '我想知道关于比奇省历史的事。'))
                    end
                end,

                -- @INVITE_GUARD3_20
                npc_guard_3_give_info = function(uid, value)
                    if not server.player.removeItem(uid, '烧酒', 5) then
                        runEventHandler(uid, questPath, SYS_ENTER)
                        return
                    end

                    if not uidRemoteCall(questUID, uid,
                    {
                        [=[<par>好吧，我来讲给你听。</par>]=],
                        [=[<par>唔<t wrap="0">···</t>这已经是我所知道的全部故事啦！就说到这里吧，酒喝得很爽啊！</par>]=],
                    },
                    [=[
                        local playerUID, texts = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_give_guard_3_soju', state='quest_guard_3_give_info', args=texts}
                    ]=]) then
                        server.player.addItem(uid, '烧酒', 5)
                    end
                end,
            }
        ]])
    end,

    quest_guard_3_give_info = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '听到了关于比奇历史的故事，如果从所有休班卫士那里听到相关内容，就赶快找图书管理员把内容转达给他。'}
        uidRemoteCall(getNPCharUID('比奇县_0', '休班卫士_3'), uid, value,
        [[
            local playerUID, texts = ...
            local dialog = require('include.dialog')

            local function unwrapPar(raw)
                if raw == nil or raw == '' then
                    return nil
                end
                local content = string.match(raw, '^<par>(.*)</par>$')
                if content == nil then
                    fatalPrintf('unexpected par format: %s', raw)
                end
                return content
            end

            local text = {}
            local text1 = unwrapPar(texts[1])
            if text1 then
                table.insert(text, text1)
            end

            table.insert(text, '祖先们修建了这比奇省和里面的城镇村庄之后，就开始反复的在周边勘查并拓展自己的根据地。但是这附近值得利用的土地非常的少。很难足够的支持别的地方的农事生产需要。')
            table.insert(text, '随着人口逐渐的增加，人们为了寻找更加宽阔的土地和更多的资源开始拓宽自己的领土。' ..
                '于是人们向沃玛、蛇谷、盟众一步一步的扩大土地，开拓没有人烟到达过的沼泽地，也遇到了生活在森林、灌木丛和山洞中其它各种各样的怪物并与它们发生战争，就这样一点一点的扩大了领土，可以说每一寸土地都是用鲜血换来的啊！')
            table.insert(text, '尽管我们现在占据了宽广的领土，但在比奇土地上各处都仍存在着怪物的势力，加上大部分地区全都是深山和茂密的灌木丛，仍然会发生种种阻断村庄之间道路的事情<t wrap="0">···</t>')

            local text2 = unwrapPar(texts[2])
            if text2 then
                table.insert(text, text2)
            end

            dialog.post(playerUID, text,
            dialog.link(SYS_EXIT, '谢谢你！'))
        ]])

        setQuestState{uid=uid, fsm=fsmName_persuade_librarian, state='quest_answer_librarian_questions'}
    end,

    quest_answer_librarian_questions = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '把休班卫士说的故事转达给图书管理员，回答他的提问。'}
        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
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
                    dialog.post(uid, questPath, '听说你已经收集到了所有关于比奇省的历史了？真是太好了！',
                    dialog.link('npc_question_1', '是的'))
                end,

                npc_question_1 = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '我有个问题不确定很久了，正好问你！',
                        '听说在比奇产生之前，西方的国家为了讨伐某种怪物派种族遣出过远征队，那种怪物是什么种族的呢？',
                    },
                    {
                        dialog.link('npc_question_2', '诺玛和内日', {args = '1'}),
                        dialog.link('npc_question_2', '沃玛和祖玛', {args = '2'}),
                        dialog.link('npc_question_2', '半兽人和内日', {args = '3'}),
                        dialog.link('npc_question_2', '半兽人和诺玛', {args = '4'}),
                    })
                end,

                npc_question_2 = function(uid, value)
                    if value == '3' then
                        local selections = shuffleArray(
                        {
                            dialog.link('npc_question_3', '祖玛教主的宫殿', {args = '1'}),
                            dialog.link('npc_question_3', '蛇的集体栖息地', {args = '2'}),
                            dialog.link('npc_question_3', '半兽人的根据地', {args = '3'}),
                            dialog.link('npc_question_3', '内日族的根据地', {args = '4'}),
                        })

                        dialog.post(uid, questPath,
                        {
                            '哦！原来还是那些家伙啊<t wrap="0">···</t>',
                            '那么在人类来此生活之前的比奇县什么样的地方呢？',
                        },
                        selections)
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                npc_question_3 = function(uid, value)
                    if value == '3' then
                        local selections = shuffleArray(
                        {
                            dialog.link('npc_done_question', '因为发生了大地震', {args = '1'}),
                            dialog.link('npc_done_question', '因为发生了大洪水', {args = '2'}),
                            dialog.link('npc_done_question', '因为发生了大饥荒', {args = '3'}),
                            dialog.link('npc_done_question', '因为发生了大瘟疫', {args = '4'}),
                        })

                        dialog.post(uid, questPath,
                        {
                            '果然！难怪半兽人那么顽固地反扑<t wrap="0">···</t>',
                            '那么我们的祖先们回不了故乡，在这个地方落脚定居的原因是什么呢？',
                        },
                        selections)
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                npc_done_question = function(uid, value)
                    if value == "1" then
                        dialog.post(uid, questPath,
                        {
                            '怪不得呢！所以越过山脉的路就被隔断了啊！',
                            '你真的是认真努力的调查过啦！托您的福，史书的撰写进度加快了！等这本书全部完成之后一定会在末尾写上你的大名的。',
                            '那么请你去把我要加入比奇商会的意思转告给王大人吧！',
                            '真是太谢谢了！',
                        })

                        uidRemoteCall(questUID, uid, getNPCMapName(false), getNPCName(false),
                        [=[
                            local playerUID, mapName, npcName = ...
                            if setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_answer_librarian_questions', state=SYS_DONE} then
                                clearNPCQuestBehavior(mapName, npcName, playerUID)

                                -- the guards go back to their own talk, legacy's replay at [146] ends with [165]
                                for _, guard in ipairs({'休班卫士_1', '休班卫士_2', '休班卫士_3'}) do
                                    clearNPCQuestBehavior('比奇县_0', guard, playerUID)
                                end
                            end
                        ]=])
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                -- @BICHUN_TEST1_4_3, set [146]
                npc_wrong_answer = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '奇怪！根据我的调查好像不是这么回事儿啊！你确定没有听错吗？',
                        '请再去打听一下吧！',
                    },
                    dialog.link(SYS_EXIT, '退出'))

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_answer_librarian_questions', state='quest_player_answer_incorrectly'}
                    ]=])
                end,
            }
        ]])
    end,

    -- [146], a wrong answer: the guards tell their stories again (@REPLAY_GUARD1..3), the librarian asks again (@BICHUN_TEST2)
    quest_player_answer_incorrectly = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_librarian, '你没能回答图书管理员的问题，再去找所有休班卫士听他们的故事吧。'}

        -- @BICHUN_TEST2 starts at the first question
        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidRemoteCall(questUID, uid, questName,
                    [=[
                        local playerUID, questName = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from='quest_player_answer_incorrectly', state='quest_answer_librarian_questions', exitfunc=function()
                            runNPCEventHandler(getNPCharUID('比奇县_0', '图书管理员_1'), playerUID, {SYS_EPUID, questName}, 'npc_question_1')
                        end}
                    ]=])
                end,
            }
        ]])

        -- @REPLAY_GUARD1
        setupNPCQuestBehavior('比奇县_0', '休班卫士_1', uid,
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
                        '不要总是在开玩笑啦！',
                        '看来你是要从我这里知道些什么吧！但是没那么容易啦！',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @REPLAY_GUARD2, the story of npc_guard_2_give_info again
        setupNPCQuestBehavior('比奇县_0', '休班卫士_2', uid,
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
                        '累得要命，你可要仔细听好并记住啊！',
                        '知道吗？我们的祖先就是讨伐半兽人族地区而派遣出的远征队啊！我们的祖先经过残酷的战斗终于击溃了怪物们。一想到只要再继续坚持战斗一下就可以把怪物们斩草除根，然后可以回到故乡，就都非常高兴。',
                        '可是没想到这时突然发生了始料未及的灾难。这里发生了大地震。原本可以翻过山脉回到家乡的路由于这次大地震导致地壳变动，完全的被隔断了！有的人痛哭流涕，有的人茫然失措。所有人都慌了手脚。',
                        '但是一位优秀的将领重新振作精神，开始在这个地区寻找求生之路。他指挥着他的部下们在赶走半兽人族的地区找到了一片肥沃的土地建立了新的城市。这就是现在的比奇省。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])

        -- @REPLAY_GUARD3: quest_guard_3_give_info again, its args are the lines around the story, it goes back to the questions
        -- installed here only, so it stays after the librarian asked again
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID()
        ]],
        [[
            local questUID = ...
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidRemoteCall(questUID, uid, {[=[<par>哦？那我就再给你讲一遍吧！</par>]=]},
                    [=[
                        local playerUID, texts = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, from={'quest_player_answer_incorrectly', 'quest_answer_librarian_questions'}, state='quest_guard_3_give_info', args=texts}
                    ]=])
                end,
            }
        ]])
    end,
})

setQuestFSMTable(fsmName_persuade_pharmacist,
{
    [SYS_DONE] = function(uid)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_pharmacist, '将毒蛇牙齿送给药剂师，药剂师同意加入比奇商会。'}
    end,
    [SYS_ENTER] = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_pharmacist, '药剂师因找不到能治疗传染病的药而苦恼，主要药材毒蛇牙齿能在蛇谷金中医那里找到。'}
        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
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
                    dialog.post(uid, questPath, dialog.link('npc_path_details', '毒蛇山谷', {prefix = '患者越来越多，快去', suffix = '买<t color="red">毒蛇牙齿</t>吧！'}),
                    dialog.link(SYS_EXIT, '好的'))
                end,

                npc_path_details = function(uid, value)
                    dialog.post(uid, questPath, '从这儿向东北部去就能到达毒蛇山谷，去(643，15)附近就能够找得到。穿过毒蛇山谷一直向东走就会达到那个村庄，在那儿找药商<t color="red">金中医</t>(334，224)向他购买<t color="red">毒蛇牙齿</t>。',
                    dialog.link(SYS_EXIT, '好的'))
                end,
            }
        ]])

        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
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
                    dialog.post(uid, questPath, '噢！是来买毒蛇牙齿的啊！',
                    dialog.link('npc_buy_tooth', '是的！'))
                end,

                npc_buy_tooth = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '唔？看起来你不是要药材商或者从医的人吧！',
                        '这倒无所谓！不过作为药用的毒蛇牙齿的产量是固定的，每天充其量能供给几包而已。难道你不知道吗？',
                    },
                    dialog.link('npc_seller_call_price', '我是受比奇省药剂师之托而来的'))
                end,

                npc_seller_call_price = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '比奇省发生传染病？你说的是真的吗？那我现在就卖给你一包吧！',
                        '现在只有这些，如果需要的话再来吧！价格是100钱一颗，给我1000钱就行。',
                    },
                    {
                        dialog.link('npc_pay_full_price', '全额付款'),
                        dialog.link('npc_ask_for_discount', '讨价还价'),
                    })
                end,

                -- @BUY_TOOTH_3, short of the gold legacy gives the teeth for free, here it goes as a short agreed price does
                npc_pay_full_price = function(uid, value)
                    if not server.player.removeGold(uid, 1000) then
                        uidRemoteCall(questUID, uid,
                        [=[
                            local playerUID = ...
                            setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from=SYS_ENTER, state='quest_purchase_with_agreed_price', args=1000}
                        ]=])
                        return
                    end

                    if not uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from=SYS_ENTER, state='quest_purchased_tooth'}
                    ]=]) then
                        server.player.addItem(uid, SYS_GOLDNAME, 1000)
                        return
                    end

                    dialog.post(uid, questPath,
                    {
                        '很着急的样子啊！',
                        '给你，快去比奇省看看吧！',
                    },
                    dialog.link(SYS_EXIT, '好的'))

                    server.player.addItem(uid, '毒蛇牙齿', 10)
                end,

                npc_ask_for_discount = function(uid, value)
                    dialog.post(uid, questPath,
                    {
                        '城内的情况这么紧急的话我就赔本卖给你吧！',
                        '每颗10钱，只付100钱就行。连加工费都去掉就按成本给你啦！',
                    },
                    dialog.link('npc_setup_purchase_quest', '好的'))
                end,

                npc_setup_purchase_quest = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from=SYS_ENTER, state='quest_purchase_with_agreed_price', args=100}
                    ]=])
                end,
            }
        ]])
    end,

    quest_purchase_with_agreed_price = function(uid, value)

        -- purchase with agreed price
        -- this state won't pop up dialog if player has enough money

        assertType(value, 'integer')
        assert(value > 0)

        uidRemoteCall(getNPCharUID('毒蛇山谷_2', '金中医_1'), uid, value, getUID(), getQuestName(),
        [[
            local playerUID, askedGold, questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local currGold = uidRemoteCall(playerUID, [=[ return getGold() ]=])
            local dialog = require('include.dialog')

            if currGold >= askedGold then
                -- the switch first, a replay of this state at login meanwhile doesn't buy twice
                if not uidRemoteCall(questUID, playerUID,
                [=[
                    local playerUID = ...
                    return setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchase_with_agreed_price', state='quest_purchased_tooth'}
                ]=]) then
                    return
                end

                dialog.post(playerUID, '东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！',
                dialog.link(SYS_EXIT, '好的'))

                uidRemoteCall(playerUID, askedGold,
                [=[
                    local askedGold = ...
                    removeItem(getItemID(SYS_GOLDNAME), 0, askedGold)
                    addItem(getItemID('毒蛇牙齿'), 10)
                ]=])

            else
                local rand = math.random(0, 100)
                if rand <= 0 then
                    uidRemoteCall(questUID, playerUID, questPath,
                    [=[
                        local playerUID, questPath = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchase_with_agreed_price', state='quest_purchase_with_free_price', exitfunc=function()
                            runNPCEventHandler(getNPCharUID('毒蛇山谷_2', '金中医_1'), playerUID, questPath, SYS_ENTER)
                        end}
                    ]=])

                elseif rand <= 50 then
                    dialog.post(playerUID,
                    {
                        string.format('你是在开玩笑吗？你没有<t color="red">%d</t>金币啊？！', askedGold),
                        string.format('你这个不老实的家伙，不要再浪费我的时间了！先凑够<t color="red">%d</t>金币再来找我吧！', askedGold),
                    },
                    dialog.link(SYS_EXIT, '退出'))

                    uidRemoteCall(questUID, playerUID, askedGold,
                    [=[
                        local playerUID, askedGold = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchase_with_agreed_price', state='quest_wait_purchase', args=askedGold}
                    ]=])

                else
                    local newAskedGold = math.ceil(askedGold * 1.5)
                    dialog.post(playerUID,
                    {
                        string.format('你是在开玩笑吗？你没有<t color="red">%d</t>金币啊？！', askedGold),
                        string.format('你这个家伙实在浪费我的一片好心，不要再说了！我决定涨价<t color="red">50%%</t>，先凑够<t color="red">%d</t>金币再来找我吧！', newAskedGold),
                    },
                    dialog.link(SYS_EXIT, '退出'))

                    uidRemoteCall(questUID, playerUID, newAskedGold,
                    [=[
                        local playerUID, newAskedGold = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchase_with_agreed_price', state='quest_wait_purchase', args=newAskedGold}
                    ]=])
                end
            end
        ]])
    end,

    quest_wait_purchase = function(uid, value)
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid, string.format([[ return %d, getUID(), getQuestName() ]], value),
        [[
            local askedGold, questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, value)
                    dialog.post(uid, questPath, string.format('你带来<t color="red">%d</t>金币了吗？', askedGold),
                    {
                        dialog.link('npc_purchase', '带来了！'),
                        dialog.link(SYS_EXIT, '我还没凑齐！'),
                    })
                end,

                npc_purchase = function(uid, value)
                    uidRemoteCall(questUID, uid, askedGold,
                    [=[
                        local playerUID, askedGold = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_wait_purchase', state='quest_purchase_with_agreed_price', args=askedGold}
                    ]=])
                end,
            }
        ]])
    end,

    quest_purchase_with_free_price = function(uid, value)
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
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
                    dialog.post(uid, questPath, '真是个难缠的家伙！拿你没办法，先免费给你快拿去给病人们治病用吧！',
                    dialog.link('npc_say_thanks', '谢谢你的慷慨！'))
                end,

                npc_say_thanks = function(uid, value)
                    if not uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchase_with_free_price', state='quest_purchased_tooth'}
                    ]=]) then
                        return
                    end

                    dialog.post(uid, questPath, '我是看药剂师的面子才免费的！东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！',
                    dialog.link(SYS_EXIT, '好的'))

                    uidRemoteCall(uid, [=[ addItem(getItemID('毒蛇牙齿'), 10) ]=])
                end,
            }
        ]])
    end,

    quest_purchased_tooth = function(uid, value)
        setQuestDesp{uid=uid, fsm=fsmName_persuade_pharmacist, '在蛇谷找到的毒蛇牙齿就是专门治疗最近流行传染病的药，快把毒蛇牙齿转给比奇药剂师吧。'}
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
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
                    dialog.post(uid, questPath, '干嘛呢？还不快把药材带给<t color="yellow">比奇省</t><t color="red">药剂师</t>。',
                    dialog.link(SYS_EXIT, '退出'))
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                -- @GIVE_TOOTH
                [SYS_ENTER] = function(uid, value)
                    if not server.player.removeItem(uid, '毒蛇牙齿', 10) then
                        dialog.post(uid, questPath,
                        {
                            '啊？怎么回事？',
                            '见了金中医为什么没带回所要的东西？',
                            '情况紧急，要抓紧时间啊！',
                        },
                        dialog.link(SYS_EXIT, '退出'))
                        return
                    end

                    if not uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, from='quest_purchased_tooth', state=SYS_DONE}
                    ]=]) then
                        server.player.addItem(uid, '毒蛇牙齿', 10)
                        return
                    end

                    -- @GIVE_TOOTH_3, checkjob warrior; legacy's text says 金创药 to everyone
                    local warrior = server.player.hasJob(uid, '战士')
                    dialog.post(uid, questPath,
                    {
                        '您为病人们做了一件大好事！所以我会听从你的劝说加入王大人的比奇商会的，只好对不起崔大夫了！',
                        string.format('啊！对了，这是%s，收下这个吧！急匆匆地走了这么远的路累坏了吧！喝了这个可以补充一下元气。', warrior and '金创药' or '魔法药'),
                    },
                    dialog.link(SYS_EXIT, '谢谢！'))

                    server.player.addItem(uid, warrior and '金创药（特）' or '魔法药（特）', 8)
                    uidRemoteCall(questUID, uid, getNPCMapName(false), getNPCName(false),
                    [=[
                        local playerUID, mapName, npcName = ...
                        clearNPCQuestBehavior(mapName, npcName, playerUID)
                    ]=])
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '王大人_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return SYS_DEBUG or uidRemoteCall(uid, [=[ return dbHasFlag('done_wang_book') ]=])
        end,

        [SYS_ENTER] = function(uid, value)
            local level = uidRemoteCall(uid, [=[ return getLevel() ]=])
            if level < 9 then
                dialog.post(uid, questPath, '我要托付你帮我办点事情，但是现在的你好象还有点应付不了。再去好好修炼一下，等级达到9级的时候才能够得到我的信任让我把这件事情交给你去做。',
                dialog.link(SYS_EXIT, '退出'))
            else
                dialog.post(uid, questPath,
                {
                    '噢！你来啦！你要是不来我正好要派人去叫你呢？其实我是有事情要拜托你做！是这样的！最近以我为会长的比奇商会和以那个姓崔的贪得无厌的家伙为首的传奇商会正在互相争夺势力。',
                    '啊！' ..
                    '当然不是真刀真枪的动武啦！' ..
                    '是为了争夺商权而展开的势力之争。' ..
                    '不管怎么样，胜者为王败者寇，在这场斗争中失败者将被排挤出比奇省。' ..
                    '因此想要求你帮我办点事情，为了增强我们比奇商会的势力，请你去说服<t color="red">图书管理人</t>和施药商<t color="red">药剂师</t>从传奇商会中退出，来加入我们比奇商会。',
                },
                {
                    dialog.link('npc_decide', '好的！', {args = 'true'}),
                    dialog.link('npc_decide', '我还有别的事情要做。', {args = 'false'}),
                })
            end
        end,

        npc_decide = function(uid, value)
            uidRemoteCall(questUID, uid, value,
            [=[
                local playerUID, accepted = ...
                setQuestState{uid=playerUID, from=SYS_LUANIL, state=SYS_ENTER, args=(accepted == 'true')}
            ]=])
        end,
    })
]])
