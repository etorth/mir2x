-- converted from Envir/QuestDiary/MU_taoist/ilgang.txt
-- with its two kill hooks, MonQuest/ilgang1.txt and MonQuest/ilgang2.txt
--
-- 清明子 sends you into 试练场_1_012 with one 半兽战士 and three 半兽人 and five minutes on the
-- clock. the 战士 is the whole test: killing it passes and puts you straight back outside. the
-- 半兽人 are only pressure, and killing one brings two more, so working through them is a trap
--
-- 1_012 has no NPC standing in it, unlike the warrior ground, so there is nobody to report to
-- and the kill itself is what ends the trial
--
-- legacy turned a second taoist away with 有人在接受测试，请等一下 because every one of them
-- shared the single map. each attempt loads its own copy here, so nobody qualified is refused

_G.minQuestLevel = 8

_G.magicName  = '精神力战法'
_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

-- MonGen 半兽战士61 1 1 / MonGen 半兽人61 3 1, both at Param1..3 = 1_012 9 12
_G.trialMap     = '试练场_1_012'
_G.trialX       = 9
_G.trialY       = 12
_G.trialMinutes = 5

-- TimeRecall 5 dumps you here, and so does the mapmove in ilgang1
_G.exitMap = '本馆_1_002'
_G.exitX   = 12
_G.exitY   = 11

-- hand the map copy back and stop the clock, in either order of finishing
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

-- ilgang1: the 半兽战士 is the test
-- ilgang2: a dead 半兽人 brings two more, and says so
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_trial' then
        return
    end

    local mapUID = dbGetQuestVar(uid, 'trialMapUID')
    if not mapUID then
        return
    end

    if getMonsterName(monsterID) == '半兽战士' then
        server.player.postString(uid, '（噢，终于通过了学习精神力战法的测试……）')
        setQuestState{uid = uid, state = 'quest_trial_passed'}

    elseif getMonsterName(monsterID) == '半兽人' then
        server.player.postString(uid, '（这么大的事情。半兽人没有了，还要再出现的……）')
        uidRemoteCall(mapUID, trialX, trialY,
        [[
            local x, y = ...
            for _ = 1, 2 do
                addMonster('半兽人', x, y, false)
            end
        ]])
    end
end)

-- logging out or dying in there ends the attempt, the map copy goes with it
local function abandonTrial(uid)
    if dbGetQuestState(uid) == 'quest_in_trial' then
        setQuestState{uid = uid, state = 'quest_ready'}
    end
end

-- and coming back finds no copy to come back to, a restart took it with it
addQuestTrigger(SYS_ON_ONLINE, abandonTrial)
addQuestTrigger(SYS_ON_OFFLINE, abandonTrial)
addQuestTrigger(SYS_ON_DIE, abandonTrial)

-- 清明子 standing outside, before the first attempt and between later ones
--
-- the retry line is his own, from the [500] branch of @mugong_ilgang: he tells you what you got
-- wrong before offering the trip again
local function setupTeacher(uid, retry)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %s ]], tostring(retry)),
    [[
        local questUID, questName, retry = ...
        local questPath = {SYS_EPUID, questName}

        local function postTrialOffer(uid)
            if retry then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>不是不管三七二十一就舞剑。<t color="red">先走心剑紧随其后。</t>保持心如止水，冷静对敌。</par>
                        <par></par>
                        <par>想重新接受修炼吗？</par>
                        <par><event id="npc_enter_trial">好的, 再拜托你一次。</event></par>
                        <par><event id="npc_not_yet">准备好了，再来！</event></par>
                    </layout>
                ]=])
            else
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>那么将我移动到<t color="red">修炼场</t>。有可能要辛苦些，请做好准备！</par>
                        <par></par>
                        <par><event id="npc_enter_trial">移 动</event></par>
                        <par><event id="npc_explain">修炼场里要做什么？</event></par>
                        <par><event id="npc_not_yet">准备好了，再来！</event></par>
                    </layout>
                ]=])
            end
        end

        return
        {
            [SYS_LABEL] = retry and '再去修炼场' or '去修炼场',
            [SYS_ENTER] = postTrialOffer,

            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想掌握精神力战法，请在一定的时间之内将训练场里的怪兽都处理了即可。</par>
                        <par></par>
                        <par><event id="npc_enter_trial">好的，拜托了！</event></par>
                        <par><event id="npc_not_yet">准备好了，再来！</event></par>
                    </layout>
                ]=])
            end,

            npc_not_yet = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>药准备多些好，那么快点去吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_enter_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- he has agreed to teach you, you are standing next to him
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '清明子答应指点你精神力战法，跟他说一声就可以进修炼场。'}
        setupTeacher(uid, false)
    end,

    -- came back out without passing, he offers another go and tells you why you failed
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '修炼场的考验还没通过，再去找本馆的清明子。'}
        setupTeacher(uid, true)
    end,

    -- checkhum/Monclear/MonGen/TimeRecall/map, all of @mugong_ilgang_next4_2 onwards
    quest_enter_trial = function(uid, args)
        local mapUID = loadInstanceMap(trialMap)
        if not mapUID then
            server.player.postString(uid, '修炼场现在进不去，过一会儿再来吧。')
            setQuestState{uid = uid, state = 'quest_ready'}
            return
        end

        uidRemoteCall(mapUID, trialX, trialY,
        [[
            local x, y = ...
            addMonster('半兽战士', x, y, false)
            for _ = 1, 3 do
                addMonster('半兽人', x, y, false)
            end
        ]])

        dbSetQuestVar(uid, 'trialMapUID', mapUID)

        -- TimeRecall 5, the trial is over whether or not the 战士 is down
        dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
            pause(trialMinutes * 60 * 1000)
            server.player.postString(uid, '时间到了，你被送出了修炼场。')
            setQuestState{uid = uid, state = 'quest_ready'}
        end))

        server.player.spaceMove(uid, mapUID, trialX, trialY)
        setQuestState{uid = uid, state = 'quest_in_trial'}
    end,

    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在修炼场里，%d 分钟内打倒半兽战士。半兽人杀了还会再来，别在它们身上耗时间。', trialMinutes}
    end,

    -- [501], and out through the same exit ilgang1 used
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '通过了修炼场的考验，回去找清明子领精神力战法秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领精神力战法秘籍',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道你可以成功。</par>
                            <par>辛苦了，<t color="red">现在可以看到一点剑路了吗？</t></par>
                            <par></par>
                            <par>这个是精神力战法要点解释的<t color="red">秘籍</t>，请拿走看看。你已经具有了基本素质，只要掌握要点，充分地可以学习精神力战法。</par>
                            <par>这里有些金币和东西，用在需要的地方。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">谢谢！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '精神力战法（秘籍）', 1)
                    server.player.addItem(uid, '灵魂铁手镯', 1)
                    server.player.deliverGold(uid, 9000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_ilgang, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- he opens with this whether or not you qualify
    local pitch = [=[精神力战法是剑术造诣很深的某个先辈故人创造的<t color="red">为了道士的剑法</t>。道士们终究是比战士们力量弱，如果不学习精神力战法，放弃<t color="red">直接进攻</t>还是好些。]=]

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼精神力战法',

        -- legacy left @mugong_ilgang clickable forever and answered out of the flags, so this
        -- stays up before the quest and after it. while it runs the EPUID behavior installed by
        -- the FSM hides this entry, npchar drops an EPQST entry whose quest has one
        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid = uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- [716], he remembers handing the book over
            if server.quest.getState(questUID, {uid = uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到<t color="red">%s秘籍</t>吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], magicName, SYS_EXIT)
                return
            end

            -- 该武功不是其它职业的人们可以掌握的简单武功呀
            if not server.player.hasJob(uid, '道士') then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>该武功不是其它职业的人们可以掌握的简单武功呀。只有<t color="red">道士</t>才可以掌握。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 精神力战法, he can see it in your eyes
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>看你的眼光很锐利，好象正在修炼<t color="red">%s</t>。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], magicName, SYS_EXIT)
                return
            end

            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>%s</par>
                        <par></par>
                        <par>嗯。。。但是你好像还没有达到修炼精神力战法的水平。在修炼一下准备好了，再来！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], pitch, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>%s</par>
                    <par></par>
                    <par>现在你已经到了该修炼精神力战法的时候，我教你修炼。和修炼其它的魔法不一样，现在是修炼剑法，所以修炼方法和战士的修炼方法没有什么不同的。</par>
                    <par></par>
                    <par>怎么样？接受修炼吗？</par>
                    <par><event id="npc_accept">好的，拜托了！</event></par>
                    <par><event id="npc_explain">修炼场里要做什么？</event></par>
                    <par><event id="npc_not_yet">准备好之后，再来！</event></par>
                </layout>
            ]=], pitch)
        end,

        -- @mugong_ilgang_explain
        npc_explain = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果想掌握精神力战法，请在一定的时间之内将训练场里的怪兽都处理了即可。</par>
                    <par></par>
                    <par><event id="npc_accept">好的，拜托了！</event></par>
                    <par><event id="npc_not_yet">准备好之后，再来！</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_ilgang_next5
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>药准备多些好，那么快点去吧！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_ilgang_next4_1, SET [500]
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么将我移动到<t color="red">修炼场</t>。有可能要辛苦些，请做好准备！</par>
                    <par></par>
                    <par><event id="%s" close="1">知道了</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
