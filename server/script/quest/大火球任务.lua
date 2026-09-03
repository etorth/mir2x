-- converted from Envir/QuestDiary/MU_wizard/fireUpbolt.txt
--
-- the wizard version of the warrior's 攻杀铁剑 loan. the 火焰沃玛 in 试练场_02_004 only die to
-- a 焱火剑, so 霹雳尊者 hands you one, and like the 攻杀铁剑 it can not be taken off again
-- (SDItem::EA_BIND). lose it and the next one costs 5000
--
-- there is a copy of him standing inside, and reporting to him with the room clear is what
-- ends the trial
--
-- flags: [752] done, [516] sword handed over, [517] trial passed
--
-- three legacy inconsistencies, all kept as the text has them:
--
--   @mugong_upfireball_next1 answers checkmagic with 你还没有掌握大火球魔法吗？, the question
--   inverted — the branch only runs once you do know it
--
--   the retry at @mugong_upfireball_retry2 says 规定时间是3分钟 while the first run says
--   5分钟, and both do TimeRecall 5. five is what the clock actually was
--
--   the retry spawns at 9,12 and uses map 02_004 for the default entry instead of the
--   25,22 / 25,8 pair the first run uses. the first run's placement is used for both
--
-- legacy turned a second wizard away with 有人正在接受测试，请等一下 because they all shared
-- the one map. each attempt loads its own copy here, so nobody qualified is refused

_G.minQuestLevel = 15

_G.magicName = '大火球'
_G.mijiName  = '大火球（秘籍）'

_G.swordName  = '焱火剑'
_G.swordPrice = 5000

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

-- Param1..3 = 02_004 25 22, and mapmove 02_004 25 8 drops you in across from them
_G.trialMap     = '试练场_02_004'
_G.trialNPC     = '霹雳尊者_1'
_G.trialX       = 25
_G.trialY       = 22
_G.startX       = 25
_G.startY       = 8
_G.trialCount   = 3
_G.trialMinutes = 5

-- mapmove 02 266 146
_G.exitMap = '银杏山谷_02'
_G.exitX   = 266
_G.exitY   = 146

local function closeTrial(uid)
    local timer = dbGetQuestVar(uid, 'trialTimer')
    if timer then
        dbSetQuestVar(uid, 'trialTimer', nil)
        closeThread(timer)
    end

    local mapUID = dbGetQuestVar(uid, 'trialMapUID')
    if mapUID then
        dbSetQuestVar(uid, 'trialMapUID', nil)
        closeInstanceMap(mapUID, exitMap, exitX, exitY)
    end
end

-- @mugong_upfireball_next7 from Monclear onwards
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '训练场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    uidRemoteCall(mapUID, trialCount, trialX, trialY,
    [[
        local count, x, y = ...
        for _ = 1, count do
            addMonster('火焰沃玛61', x, y, false)
        end
    ]])

    dbSetQuestVar(uid, 'trialMapUID', mapUID)

    -- @upfireball_test, the copy of him inside counts this copy's monsters
    setupInstanceNPCBehavior(mapUID, trialNPC, uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '训练场',
            [SYS_ENTER] = function(uid, value)
                if uidRemoteCall(getMapUID(), [=[ return getMonsterCount() ]=]) > 0 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请将这里所有的怪物都处理了吧！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>这里所有的怪物都被处理了嘛。。</par>
                        <par>能力还不错。。。</par>
                        <par>请在外面观看。..</par>
                        <par></par>
                        <par><event id="npc_leave_trial" close="1">关闭</event></par>
                    </layout>
                ]=])
            end,

            -- @upfireball_test_next1, SET [517]
            npc_leave_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_trial_passed'})
            end,
        }
    ]])

    -- TimeRecall 5
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了训练场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.mapUIDMove(uid, mapUID, startX, startY)
    setQuestState{uid = uid, state = 'quest_in_trial'}
end

local function abandonTrial(uid)
    if dbGetQuestState(uid) == 'quest_in_trial' then
        setQuestState{uid = uid, state = 'quest_ready'}
    end
end

addQuestTrigger(SYS_ON_ONLINE, abandonTrial)
addQuestTrigger(SYS_ON_OFFLINE, abandonTrial)
addQuestTrigger(SYS_ON_DIE, abandonTrial)

-- @mugong_upfireball_next5 and the @mugong_upfireball_retry2 that repeats it, both of which
-- start by looking at what you are holding. retry adds the line about not giving up on you
local function setupTeacher(uid, retry)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %d, %s ]], swordPrice, tostring(retry)),
    [[
        local questUID, questName, swordPrice, retry = ...
        local questPath = {SYS_EPUID, questName}

        local function wearingSword(uid)
            local item = server.player.getWLItem(uid, WLG_WEAPON)
            return item ~= nil and item.itemID == getItemID('焱火剑')
        end

        -- @upfireball_next6_1 and @upfireball_retry3_1, the same offer down to a word each
        local function postSellSword(uid)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>焱火剑丢失了？ 因此不能修炼大火球。。</par>
                    <par>如果是这样，请使用我的焱火剑吧。。但是不能白给你。。</par>
                    <par>这把剑<t color="red">%d</t>两。。那么你%s买吗？</par>
                    <par></par>
                    <par><event id="npc_buy_sword">即使贵，也要买。</event></par>
                    <par><event id="%s" close="1">由于钱不够，%s不能买。</event></par>
                </layout>
            ]=], swordPrice, retry and '还' or '', SYS_EXIT, retry and '' or '还')
        end

        local function postTrialOffer(uid)
            if not wearingSword(uid) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>焱火剑如何？只有携带焱火剑来，才可以进训练场。</par>
                        <par></par>
                        <par><event id="npc_lost_sword">由于失误，丢失了焱火剑。</event></par>
                        <par><event id="npc_explain">这个测试是怎么进行的？</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            if retry then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>使用特殊加工成锋利的剑进行实战，精神就会集中在剑头部。希望我们可以再见面。咯咯</par>
                        <par>哦，规定时间是3分钟。。希望你在规定的时间内一定可以成功。。。</par>
                        <par></par>
                        <par><event id="npc_enter_trial">下一步</event></par>
                        <par><event id="npc_explain">这个测试是怎么进行的？</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果使用特殊加工成、锋利的剑进行实战，精神都将集中在剑头部。希望我们可以再见面。咯咯</par>
                    <par>哦，规定的时间是<t color="red">5分钟</t>。。希望你在规定的时间内可以成功。。。</par>
                    <par></par>
                    <par><event id="npc_enter_trial">移 动</event></par>
                    <par><event id="npc_explain">这个测试是怎么进行的？</event></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end

        return
        {
            [SYS_LABEL] = retry and '再进训练场' or '进训练场',

            -- @mugong_upfireball_retry1 comes first on a retry
            [SYS_ENTER] = function(uid, value)
                if not retry then
                    postTrialOffer(uid)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我理解一定要掌握火焰攻击的原因是如果不这样，在学习大火球时很容易走火入魔。请静下心来，再次接受测试，一定要争取通过。</par>
                        <par></par>
                        <par><event id="npc_retry">知道了，请再次一次吧</event></par>
                        <par><event id="npc_giveup">现在我的能力好像还不够。</event></par>
                    </layout>
                ]=])
            end,

            npc_retry = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>年轻人充满欲望的脸上，让人看起来很高兴。不管帮助你多少次，一定要使你通过测试。</par>
                        <par></par>
                        <par><event id="npc_trial_offer">下一步</event></par>
                    </layout>
                ]=])
            end,

            npc_trial_offer = postTrialOffer,

            -- @mugong_upfireball_giveup
            npc_giveup = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>哦，年轻人如此没有自信心。。毫无疑问大火球是功力强大的魔法，但我看你过不去那个位置。请好好想想，再来接受测试！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_upfireball_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>为了学习大火球魔法，带上我给你的<t color="red">焱火剑</t>，然后在规定的时间内将训练场内的怪物都打倒。</par>
                        <par>焱火剑的特性是佩戴上一次后，自己就不会脱落。但是在昏迷或者失去耐久性的 时 候，才可以摘下来。</par>
                        <par>为了通过测试一定要佩戴焱火剑，如果丢失了，请花钱买！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_lost_sword = postSellSword,

            -- @upfireball_next6_2 and @upfireball_retry3_2, checkgold 5000
            npc_buy_sword = function(uid, value)
                if not server.player.removeGold(uid, swordPrice) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你没钱，还要焱火剑？如果你再讲一次，我就不再卖焱火剑了。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>在这里，焱火剑。。。注意不要丢失了%s。。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], retry and '' or '哦', SYS_EXIT)

                server.player.addBoundItem(uid, '焱火剑')
            end,

            npc_enter_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- SET [516], the sword is yours and the trial is open
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '霹雳尊者给了你一把焱火剑，戴上它去找他进训练场。'}
        setupTeacher(uid, false)
    end,

    -- back out without clearing it
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '训练场的测试还没通过，戴好焱火剑再去找霹雳尊者。'}
        setupTeacher(uid, true)
    end,

    quest_enter_trial = function(uid, args)
        enterTrial(uid)
    end,

    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在训练场里，%d 分钟内用焱火剑打倒所有火焰沃玛，然后找里面的霹雳尊者。', trialMinutes}
    end,

    -- [517]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '通过了测试，回去找霹雳尊者领大火球秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领大火球秘籍',

                -- @mugong_upfireball_complete
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，成功了吗？现在已经熟练了火焰攻击，使用大火球一样的强大魔法也不会出现走火入魔的事情了。</par>
                            <par></par>
                            <par><event id="npc_take_book">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_upfireball_give1, takew 焱火剑 1 — the loan comes back, and EA_BIND
                -- means this is the one thing that can take it off
                npc_take_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在按照约定将剩余的部分传授给你。你修炼的过程中，我将在你<t color="red">大火球秘籍</t>内贴上详细地说明，请拿走该书用心地练习吧！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeWearItem(uid, WLG_WEAPON)
                    server.player.addItem(uid, '大火球（秘籍）', 1)
                    server.player.deliverGold(uid, 13000)
                    server.player.addItem(uid, '焰火项链', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_upfireball, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼大火球',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [752] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到大火球秘籍了吗？ 那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 15
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>大火球是<t color="red">强化了的火球术</t>。正如它的名字一样是可以放出将金刚石熔化<t color="red">强大火团的技法</t>。如果掌握了第2阶段的火球，进一步修炼大火球还是比较好。</par>
                        <par>但是你现在好像还没有到可以学习的时候。做好学习准备时，请再来！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_upfireball_next1, checkmagic 大火球
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>想修炼大火球魔法吗？</par>
                        <par>你还没有掌握大火球魔法吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_upfireball_next2
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想修炼大火球魔法吗？</par>
                    <par>嘿嘿，知道了。这样的话，我就告诉你<t color="red">修炼大火球的方法</t>。大火球是将<t color="red">强大的火团射向敌人的魔法</t>，除去威力比较大之外，同火球没有很大的差异。</par>
                    <par>但是威力大正是问题的所在。因为发动者要忍耐是火球几倍的巨大的热量。</par>
                    <par></par>
                    <par><event id="npc_ask_how">没有什么可行的办法吗?</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_upfireball_next3
        npc_ask_how = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果你意如此，我向你推荐比较合适的<t color="red">训练场所</t>。那里有只有使用特殊武器才可以杀死的<t color="red">火焰系列怪物</t>，对修炼火属性武功有帮助。</par>
                    <par>你将训练场内所有怪物都扫荡的话，我认为你将对火的魔法有所熟悉，因此可以忍耐大火球魔法。其余的<t color="red">要诀</t>到时在告诉你。</par>
                    <par>现在就去训练场吗？</par>
                    <par></par>
                    <par><event id="npc_accept">好的，请将我送去吧！</event></par>
                    <par><event id="npc_not_yet">现在好像还有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_upfireball_next4_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>使用火球可以忍耐的时间是有限度的，还是要快些接受该测试吧。</par>
                    <par>如果准备好的话，请再来！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_upfireball_next4_1, give 焱火剑 1 and SET [516]. the first one is free, only
        -- a replacement costs anything
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果再讲一次，那地方的所有火焰系列怪物只能用<t color="red">焱火剑</t>杀死。这里有焱火剑，<t color="red">请带上再来。</t></par>
                    <par>同时在训练场可以停留的时间是有限制的，因此不要吝惜创伤药，请速战速决！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.player.addBoundItem(uid, '焱火剑')
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
