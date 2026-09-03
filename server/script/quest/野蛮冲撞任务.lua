-- converted from Envir/QuestDiary/MU_warrior/mute.txt
-- with its monster hook, MonQuest/mute.txt, registered in Envir/MapQuest.txt against
-- 诺玛法老 in the desert
--
-- the widest of the skill quests: twelve NPCs, and it does not start at a teacher. any of ten
-- shopkeepers will look at your battered gear, remark on it, and mention that a 黄河大侠 near
-- 边境城市 knows a way out of being surrounded — the weapon sellers say it one way and the
-- clothiers another
--
-- 黄河大侠 then sends you across the desert to 王铁匠 in 绿洲 with a letter, and the letter
-- turns out to say nothing: crossing the desert was the training. 王铁匠 asks for five 诺玛石
-- off the 诺玛法老 for an old wound, pays you in 诺玛重盔甲, and sends a letter back
--
-- flags: [704] done, [508] heard about 黄河大侠, [509] carrying his letter, [510] 王铁匠 asked
-- for the stones, [511] five stones in hand, [512] 王铁匠 paid and wrote back
--
-- three things about the legacy data:
--
--   @mugong_mute_explan_armor_m is a whole third version of the clothier's talk that no NPC
--   ever calls, so it is dead there. its first line is used as the second option on the
--   clothier's version here, which is the only difference between the two
--
--   two places call 黄河大侠 皇甫 instead. kept as written
--
--   the drop is hooked on legacy maps 41, 42, 43, 44, 4 and 6. mir2x has 诺玛村庄_41, 绿洲_4
--   and 沙漠_6, and no counterpart for 42, 43 or 44, so those three are simply not there

_G.minQuestLevel = 27

_G.magicName = '野蛮冲撞'
_G.mijiName  = '野蛮冲撞（秘籍）'

_G.letterName = '书信'
_G.stoneName  = '诺玛石'
_G.stoneCount = 5

_G.teacherMap = '边境城市_01'
_G.teacherNPC = '黄河大侠_1'

_G.smithMap = '武器店_4_001'
_G.smithNPC = '王铁匠_1'

-- random 2 in MonQuest/mute.txt
_G.stoneChance = 2
_G.stoneMaps   = {'诺玛村庄_41', '绿洲_4', '沙漠_6'}

-- @mugong_mute_explan_mugi, the five who sell weapons
_G.weaponShops =
{
    {'比奇县_0', '老张_1'},
    {'边境城市_01', '德秀_1'},
    {'银杏山谷_02', '铁匠师傅_1'},
    {'道馆_1', '铁匠_1'},
    {'武器仓库_1_001', '阿潘_1'},
}

-- @mugong_mute_explan_armor, the five who sell what you wear
_G.armorShops =
{
    {'比奇县_0', '怡美_1'},
    {'边境城市_01', '顺子_1'},
    {'银杏山谷_02', '布店晓芙_1'},
    {'道馆_1', '梁生_1'},
    {'洗衣住居_1_003', '阿浩_1'},
}

-- the [508] line every one of the ten falls back to once you have heard it
local function setupShopNag(uid)
    for _, shopList in ipairs({weaponShops, armorShops}) do
        for _, shop in ipairs(shopList) do
            setupNPCQuestBehavior(shop[1], shop[2], uid,
            [[
                return getQuestName()
            ]],
            [[
                local questName = ...
                local questPath = {SYS_EPUID, questName}

                return
                {
                    [SYS_LABEL] = '野蛮冲撞的事',
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>叫野蛮冲撞的武功请找黄河大侠。。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                }
            ]])
        end
    end
end

-- @mugon_mutebo_retry and @mugong_mutebo_test_complete_retry, the same two lines from both
-- ends of the desert: get moving, or here is another letter
local function letterRetryHandlers()
    return
    [[
        [SYS_ENTER] = function(uid, value)
            if server.player.hasItem(uid, '书信', 1) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>沙漠是很远的路。快点！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>书信丢了？让人寒心！重新再给你一本，这次注意拿好。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.player.addItem(uid, '书信', 1)
        end,
    ]]
end

setQuestFSMTable(
{
    -- set [508], one of the ten has pointed you at 黄河大侠
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '去边境城市找黄河大侠，问野蛮冲撞的事。'}
        setupShopNag(uid)

        -- the [508] branch of @mugong_mutebo and everything after it
        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '问野蛮冲撞',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，年纪轻轻好像有相当实力的武功。现在还有这样的战士，找我有什么事情吗？</par>
                            <par></par>
                            <par><event id="npc_ask_magic">想了解新的武功。</event></par>
                            <par><event id="npc_explain">这件事要怎么做？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>为了学习野蛮冲撞，首先要把我给你的<t color="red">书信</t>转交给<t color="red">绿树村的王铁匠</t>，然后接受王铁匠的一个委托。</par>
                            <par>听说要从诺玛法老处找到<t color="red">诺玛石</t>5个左右。如果诺玛石都找到了，请重新将一个<t color="red">书信</t>转交给我。</par>
                            <par>将那个书信 拿给我即可。</par>
                            <par></par>
                            <par><event id="npc_ask_magic">想了解新的武功。</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_mutebo_next
                npc_ask_magic = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯，你好像在实战中也有些体会。虽然战士总是想在最前方战斗，但是没有这种<t color="red">护身术</t>。魔法师可以利用瞬息移动魔法消失掉，道士也可以利用隐身术隐藏起自己的行踪，我们只有将敌人打倒后才可以脱身。如果被层层包围，真是死路一条。我也是经历了无数的生死考验，真是为了解决战士的困难才创造了<t color="red">野蛮冲撞</t>。</par>
                            <par></par>
                            <par><event id="npc_what_does_it_do">野蛮冲撞是具有哪种功能的武功？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_next1
                npc_what_does_it_do = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>可以推开敌人，是一种简单而实用的武功。虽然表面看起来仅仅凭借力量将敌人推开，但不是使用肌肉的力量，而是集中<t color="red">内力和外力</t>达到极限的高级武功。如果熟练地掌握了该武功，可以将比自身大几倍的巨物一下子推开。</par>
                            <par>虽然不能给敌人更大的打击，在被敌人包围的状况下可以打出一条<t color="red">血路</t>。对于在最前方和敌人正面战斗的战士来说是非常重要的武功。</par>
                            <par>但是也不能认为该武功是简单的推挡技术。根据使用者的不同，可以作为<t color="red">避免魔法或者连续器</t>使用，达到各种各样效果潜在力非常大的武功。</par>
                            <par></par>
                            <par><event id="npc_teach_me">请传授野蛮冲撞武功！</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_next2
                npc_teach_me = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好的，我的修炼方法非常严格。如果按照此方法学习，我将传授野蛮冲撞给你。</par>
                            <par></par>
                            <par><event id="npc_take_letter">我应做的事情是什么？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_next3, give 书信 and SET [509]. its checkbaggage turns you
                -- away with 你的背囊装满了。。请整理些位置再来！, and mir2x has no inventory-full
                -- check to hang that on
                npc_take_letter = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>带着这个<t color="red">书信</t>，穿越沙漠。找到隐居在<t color="red">绿洲村</t>叫<t color="red">‘王铁匠’</t>的武士，并将书信交给他，他就会告诉你某种秘诀。接受他的指教后再来！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '书信', 1)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_carry_letter'})
                end,
            }
        ]])
    end,

    -- SET [509], his letter is in your pack and the desert is ahead
    quest_carry_letter = function(uid, args)
        setQuestDesp{uid=uid, '带着黄河大侠的书信穿过沙漠，交给绿洲村的王铁匠。'}
        setupShopNag(uid)

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        string.format(
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '书信的事',
                %s
            }
        ]], letterRetryHandlers()))

        -- @mugong_mutebo_test, 王铁匠 taking the letter
        setupNPCQuestBehavior(smithMap, smithNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交书信',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>黄河大侠让来的？快点拿来书信。</par>
                            <par></par>
                            <par><event id="npc_give_letter">给你书信。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next, take 书信 and SET [510]
                npc_give_letter = function(uid, value)
                    if not server.player.hasItem(uid, '书信', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>没有书信？嗯，那就有些困难。虽然辛苦，请重新拿书信来！</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯，是黄河大侠派来的。让你来辛苦了。那么快点回去吧！</par>
                            <par></par>
                            <par><event id="npc_what">这是什么话？</event></par>
                        </layout>
                    ]=])

                    server.player.removeItem(uid, '书信', 1)
                end,

                -- @mugong_mutebo_test_next1
                npc_what = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这是你接受的全部测试。黄河大侠 每年将 几名战士 送到我这里 ，我看到书信后，将你们重新送回去即可。</par>
                            <par></par>
                            <par><event id="npc_still_angry">还是无法接受哦。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next2
                npc_still_angry = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，好像发了好大的火。但是请别误解，黄河大侠不会拿你开玩笑的。原来学习武功的方法有数十数百种。让你做这种事情都是有缘由的，不要随意轻举妄动。</par>
                            <par></par>
                            <par><event id="npc_ok">知道了。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next3
                npc_ok = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那件事就那样了，我想委托你一个<t color="red">个人的委托</t>好吗？</par>
                            <par></par>
                            <par><event id="npc_what_favor">什么委托？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next4
                npc_what_favor = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>最近年轻时候受的伤又发作了，即疼痛又很痒，都无法睡觉。听说生活在沙漠中的诺玛族拥有一种有着神奇力量称为<t color="red">诺玛石</t>的石头，将这个石头捣碎，然后用水冲服可以治疗痼疾。你能帮我找到这个东西吗？</par>
                            <par></par>
                            <par><event id="npc_no_worry">不用担心！</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next5
                npc_no_worry = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>非常谢谢！‘诺玛石’被装饰于诺玛族长老<t color="red">诺玛法老的手杖</t>上。</par>
                            <par></par>
                            <par><event id="npc_next">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next6, where he calls 黄河大侠 皇甫
                npc_next = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>非常急迫地想掌握新武功吧，别担心！即使那样，我怎么报复？向皇甫挑起事端吗？请放心地去吧！我绝对不是一个小气的家伙。</par>
                            <par></par>
                            <par><event id="npc_accept">我将给你找来。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_next7, SET [510]
                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真的吗？哦，绝对不是故意如此的。‘诺玛石’被装饰于诺玛法老的手杖上，而且请找到该<t color="red">诺玛石 5个</t>。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = 'quest_find_stones'})
                end,
            }
        ]])
    end,

    -- SET [510], off after the 诺玛法老
    quest_find_stones = function(uid, args)
        setQuestDesp{uid=uid, '在沙漠里打诺玛法老，凑齐五个诺玛石交给王铁匠。'}
        setupShopNag(uid)

        setupNPCQuestBehavior(smithMap, smithNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            -- @mugong_mutebo_test, the [510] branch, and the ELSESAY of the complete block
            local function postWaiting(uid, withCount)
                if withCount then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>需要的东西是诺玛法老出产的<t color="red">诺玛石 5个</t>，千万记住！</par>
                            <par>我将在此等候你回来。</par>
                            <par></par>
                            <par><event id="npc_explain">这件事要怎么做？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我将在此等候你回来。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end

            return
            {
                [SYS_LABEL] = '交诺玛石',

                -- @mugong_mutebo_test_complete
                [SYS_ENTER] = function(uid, value)
                    if not server.player.hasItem(uid, '诺玛石', 5) then
                        postWaiting(uid, true)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，找到‘诺玛石’了。谢谢！今天晚上开始可以好好地睡觉了。</par>
                            <par></par>
                            <par><event id="npc_hand_over">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>为了学习野蛮冲撞，首先要把我给你的<t color="red">书信</t>转交给<t color="red">绿树村的王铁匠</t>，然后接受王铁匠的一个委托。</par>
                            <par>听说要从诺玛法老处找到<t color="red">诺玛石</t>5个左右。如果诺玛石都找到了，请重新将一个<t color="red">书信</t>转交给我。</par>
                            <par>将那个书信 拿给我即可。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_mutebo_test_complete_next, SET [512], then next2_1 or next2_2 on
                -- gender for the armour and next3 to close
                npc_hand_over = function(uid, value)
                    if not server.player.hasItem(uid, '诺玛石', 5) then
                        postWaiting(uid, false)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>非常谢谢！你是一名不忽视别人困难，热心肠的人。希望你以后依然可以不断地帮助有困难的人。</par>
                            <par>这个是对你善意的小小答谢。请将书信转交给<t color="red">皇甫</t>。</par>
                            <par></par>
                            <par><event id="npc_take_armor">好的，我将转交。</event></par>
                        </layout>
                    ]=])

                    server.player.addItem(uid, '书信', 1)
                end,

                npc_take_armor = function(uid, value)
                    if not server.player.hasItem(uid, '诺玛石', 5) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>就到这里，请上路吧！要走的路还很远哟。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '诺玛石', 5)
                    server.player.addItem(uid, server.player.getGender(uid) and '诺玛重盔甲（男）' or '诺玛重盔甲（女）', 1)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_carry_reply'})
                end,
            }
        ]])
    end,

    -- SET [512], his reply is in your pack and the desert is ahead again
    quest_carry_reply = function(uid, args)
        setQuestDesp{uid=uid, '带着王铁匠的书信回边境城市，交给黄河大侠。'}
        setupShopNag(uid)

        setupNPCQuestBehavior(smithMap, smithNPC, uid,
        [[
            return getQuestName()
        ]],
        string.format(
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '书信的事',
                %s
            }
        ]], letterRetryHandlers()))

        -- @mugong_mutebo_test_complete_receive
        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交书信',
                [SYS_ENTER] = function(uid, value)
                    -- the ELSESAY here is the same 沙漠是很远的路 he uses on the way out
                    if not server.player.hasItem(uid, '书信', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>沙漠是很远的路。快点！</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>已经转交了书信？辛苦了。</par>
                            <par></par>
                            <par><event id="npc_ask_why">下一步</event></par>
                        </layout>
                    ]=])

                    server.player.removeItem(uid, '书信', 1)
                end,

                -- @mugong_mutebo_test_complete_receive_next
                npc_ask_why = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去沙漠走一趟如何？理解我为什么让你做这件事情吗？</par>
                            <par></par>
                            <par><event id="npc_no_idea">嗯，没理解。</event></par>
                            <par><event id="npc_got_it">嗯，好像理解了。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_complete_receive_next1_1
                npc_no_idea = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如此愚钝的人，到现在为止心理都在骂我吧。学习野蛮冲撞需要强大的力量和良好的内力，以及在非常艰苦的境况下也不放弃的体力和精力。为了培养这些功力，身体要处于极限的状态。因此让你横跨沙漠。</par>
                            <par></par>
                            <par><event id="npc_what">这是什么话？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_complete_receive_next1_2
                npc_got_it = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>比看起来理解快嘛。有一种将来可以成功的预感。</par>
                            <par></par>
                            <par><event id="npc_what">这是什么话？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_complete_receive_next2
                npc_what = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很正确哟。请拿着训练书，以后要帮助有困难的人。</par>
                            <par></par>
                            <par><event id="npc_take_book">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mutebo_test_complete_receive_next3, SET [704]
                npc_take_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>在哪儿、写了些什么？嗯，说你是不顾各种危险并找到药的优秀年轻人。对的，帮助有困难的人是我们有能力的人应该做的事情。非常好！你的行为提高了战士的声誉。</par>
                            <par>像你一样的人，我也相信，可以将技术传授给你。</par>
                            <par>你已经在其它地方得到了武功密集，我也没有再给你的必要了。我给你一些金币和东西，用在需要的地方。</par>
                            <par>希望以后你多做有助于提高战士名誉的事情。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '野蛮冲撞（秘籍）', 1)
                    server.player.deliverGold(uid, 30000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- MonQuest/mute.txt, one 诺玛法老 in two while [510] is set. legacy has a line per stone and
-- the fifth one is the last, so one drop rule per step
local mondrop = require('quest.include.mondrop')

local stoneLines =
{
    '(这个是诺玛石吗？。。。现在找到1个。)',
    '(现在剩下3个诺玛石了。。。)',
    '(再找到2个诺玛石就可以了。。。)',
    '(再找到1个诺玛石就可以了。。。)',
    '(诺玛石都找到了，现在该快点回去了。。)',
}

local dropList = {}
for step = 1, stoneCount do
    table.insert(dropList,
    {
        monster  = '诺玛法老',
        map      = stoneMaps,
        state    = 'quest_find_stones',
        chance   = stoneChance,

        -- the step's own line only comes out when this is the stone you are missing, which is
        -- what legacy's descending checkitem chain does
        need     = (step > 1) and {stoneName, step - 1} or nil,
        give     = stoneName,
        say      = stoneLines[step],
    })
end

-- legacy checks the highest count first and stops there, so reverse and let mondrop take the
-- first that fires
for i = 1, #dropList // 2 do
    dropList[i], dropList[#dropList + 1 - i] = dropList[#dropList + 1 - i], dropList[i]
end

mondrop.setDropOnKill(dropList)

-- and the [511] branch, which just tells you to get moving
mondrop.setDropOnKill
{
    {
        monster = '诺玛法老',
        map     = stoneMaps,
        state   = 'quest_carry_reply',
        say     = '(要快点回去了。。。)',
    },
}

-- @mugong_mutebo, the entry 黄河大侠 offers. he will not open up until a shopkeeper has
-- mentioned him, which is the ELSESAY of his [508] check
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼野蛮冲撞',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [704] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书吗？那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- the ELSESAY of check [508] 1: nobody has told you about him yet, so he sends you
            -- to get your gear seen to, which is exactly where you will hear about him
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯。。战士的路即危险又艰辛。从你所带的工具看好像经历了无数的搏斗和考验。。</par>
                    <par>首先将所持的武器刀刃磨光，每个村庄都有加工武器的商人，请他们给修理一下。那些人也许不知道你的心情。。。</par>
                    <par>以后找机会再来！</par>
                    <par></par>
                    <par><event id="%s" close="1">好的，我知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])

-- the ten shopkeepers. @mugong_mute_explan_mugi for the weapon sellers and
-- @mugong_mute_explan_armor for the clothiers, both gated on checklevel 27
local shopSetup =
{
    {
        list = weaponShops,
        code =
        [[
            local questUID, questName, minQuestLevel = ...
            local questPath = {SYS_EPQST, questName}

            setQuestHandler(questName,
            {
                [SYS_LABEL] = '聊聊装备',

                [SYS_CHECKACTIVE] = function(uid)
                    if not server.player.hasJob(uid, '战士') then
                        return false
                    end

                    if server.player.getLevel(uid) < minQuestLevel then
                        return false
                    end

                    return server.quest.getState(questUID, {uid=uid}) == nil
                end,

                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵，好久没有看到损伤这么严重的兵器了哦。究竟是进行了多么艰辛的打斗真是无法想象。这样的打斗有可能会死，请小心！随着时间的流失，熟悉的面孔一个个都消失了，让人很伤心哪。</par>
                            <par></par>
                            <par><event id="npc_frontline">谢谢！战士无论是活着还是死了，总是在战场的最前方。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_mugi_next
                npc_frontline = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很悲壮的话哦。即使是这样也是毫无办法的。希望你平安无事！哦。。听说战士的武功中有可以使战士摆脱死亡境地的武功，你知道吗？</par>
                            <par></par>
                            <par><event id="npc_never_heard">没有，第一次听说。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_mugi_next1
                npc_never_heard = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我可以帮忙噢。据说生活在边境村附近的<t color="red">黄河大侠</t>懂得该武功。请到那儿去接受指教。</par>
                            <par></par>
                            <par><event id="npc_accept">得去找黄河大侠。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_mugi_next2, set [508]
                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>坚持活下去是非常重要的。如果活着，总会实现自己的理想。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
                end,
            })
        ]],
    },

    {
        list = armorShops,
        code =
        [[
            local questUID, questName, minQuestLevel = ...
            local questPath = {SYS_EPQST, questName}

            setQuestHandler(questName,
            {
                [SYS_LABEL] = '聊聊装备',

                [SYS_CHECKACTIVE] = function(uid)
                    if not server.player.hasJob(uid, '战士') then
                        return false
                    end

                    if server.player.getLevel(uid) < minQuestLevel then
                        return false
                    end

                    return server.quest.getState(questUID, {uid=uid}) == nil
                end,

                -- the opening is shared with @mugong_mute_explan_armor_m, which nothing calls.
                -- its answer is offered here as the second option
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，防御工具被破坏的很严重嘛！看起来进行了一场非常激烈的厮杀。嗯，战士强壮虽然很有魅力，但也使人担心。如果被包围了，不是要危及到生命嘛。听说战士的武功中有可以在摆脱危机的时候使用的武功。。你知道该武功吗？</par>
                            <par></par>
                            <par><event id="npc_never_heard">没有，第一次听说。</event></par>
                            <par><event id="npc_frontline">感谢你的好意，但是战士不管生死都要在最前方。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_armor_m_next, which is the weapon seller's line
                npc_frontline = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很悲壮的话哦。即使是这样也是毫无办法的。希望你平安无事！哦。。听说战士的武功中有可以使战士摆脱死亡境地的武功，你知道吗？</par>
                            <par></par>
                            <par><event id="npc_heard_in_tavern">没有，第一次听说。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_armor_next, the one about the tavern
                npc_never_heard = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我以前在酒家听说的，说叫<t color="red">黄河大侠</t>的人懂得被敌人包围时可以逃脱的武功。好像生活在边境村附近？已经喝醉的时候听说的，现在有些想不起来了。</par>
                            <par>不是，只喝醉了一点点儿。我即使喝一杯也要醉的。真的不能喝酒。请别误会！</par>
                            <par></par>
                            <par><event id="npc_thanks">谢谢帮忙！</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_armor_m_next1, the plainer version
                npc_heard_in_tavern = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我可以帮忙噢。据说生活在边境村附近的<t color="red">黄河大侠</t>懂得该武功。请到那儿去接受指教。</par>
                            <par></par>
                            <par><event id="npc_accept">得去找黄河大侠。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_mute_explan_armor_next1, set [508]
                npc_thanks = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不会的。我们很高兴可以帮助保护我们的战士，千万要小心身体！</par>
                            <par>真是非常困难的时期啊。由于怪兽，我们都不能在野外约会。。。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
                end,

                -- @mugong_mute_explan_armor_m_next2, set [508]
                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>坚持活下去是非常重要的。如果活着，总会实现自己的理想。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
                end,
            })
        ]],
    },
}

for _, setup in ipairs(shopSetup) do
    for _, shop in ipairs(setup.list) do
        uidRemoteCall(getNPCharUID(shop[1], shop[2]), getUID(), getQuestName(), minQuestLevel, setup.code)
    end
end
