-- converted from Envir/QuestDiary/MU_wizard/lightRecall.txt
-- with its two kill hooks, MonQuest/lightRecall1.txt and MonQuest/lightRecall2.txt,
-- registered in Envir/MapQuest.txt against 试练场_02_006
--
-- 霹雳尊者 puts you in 试练场_02_006 with five minutes and a room of lightning-using undead.
-- he tells you the point is to work out what to hit first, and he means it: the 僧侣僵尸 is the
-- whole test, and killing one of the 僵尸4 or 掷斧骷髅 instead brings more of them back
--
-- 02_006 has no NPC standing in it, so the kill itself is what ends the trial
--
-- flags: [753] done, [518] trial started, [519] trial passed
--
-- legacy turned a second wizard away with 已经有人在接受考验。。。请等一下 because they all
-- shared the one map. each attempt loads its own copy here, so nobody qualified is refused

_G.minQuestLevel = 16

_G.magicName = '雷电术'
_G.mijiName  = '雷电术（秘籍）'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

-- Param1..3 = 02_006 25 22, and mapmove 02_006 25 8 puts you in across the room from them
_G.trialMap     = '试练场_02_006'
_G.trialX       = 25
_G.trialY       = 22
_G.startX       = 25
_G.startY       = 8
_G.trialMinutes = 5

-- the boss is the objective, the rest are noise
_G.bossName = '僧侣僵尸'

_G.trialSpawns = {{'僧侣僵尸', 1}, {'僵尸4', 3}, {'掷斧骷髅', 2}}

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

-- @mugong_lightstick_next4_2 from Monclear onwards
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '训练场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    uidRemoteCall(mapUID, trialSpawns, trialX, trialY,
    [[
        local spawnList, x, y = ...
        for _, entry in ipairs(spawnList) do
            for _ = 1, entry[2] do
                addMonster(entry[1], x, y, false)
            end
        end
    ]])

    dbSetQuestVar(uid, 'trialMapUID', mapUID)

    -- TimeRecall 5
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了训练场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.spaceMove(uid, mapUID, startX, startY)
    setQuestState{uid = uid, state = 'quest_in_trial'}
end

-- lightRecall1: the 僧侣僵尸 ends it
-- lightRecall2: killing anything else brings more back, but only while two or more are still
-- standing — clear it down far enough and the room stops refilling
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_trial' then
        return
    end

    local mapUID = dbGetQuestVar(uid, 'trialMapUID')
    if not mapUID then
        return
    end

    if getMonsterName(monsterID) == bossName then
        server.player.postString(uid, '（嘿，终于通过了学习雷电术的测试。。。）')
        setQuestState{uid = uid, state = 'quest_trial_passed'}
        return
    end

    -- checkmonmap 02_006 2
    if uidRemoteCall(mapUID, [[ return getMonsterCount() ]]) < 2 then
        return
    end

    server.player.postString(uid, '哦。。。（这家伙，在瞎说。好像出现了什么失误。。）')

    -- random 2, either three more zombies and one thrower or just two throwers
    local refill = (math.random(2) == 1) and {{'僵尸4', 3}, {'掷斧骷髅', 1}} or {{'掷斧骷髅', 2}}

    uidRemoteCall(mapUID, refill, trialX, trialY,
    [[
        local spawnList, x, y = ...
        for _, entry in ipairs(spawnList) do
            for _ = 1, entry[2] do
                addMonster(entry[1], x, y, false)
            end
        end
    ]])
end)

local function abandonTrial(uid)
    if dbGetQuestState(uid) == 'quest_in_trial' then
        setQuestState{uid = uid, state = 'quest_ready'}
    end
end

addQuestTrigger(SYS_ON_ONLINE, abandonTrial)
addQuestTrigger(SYS_ON_OFFLINE, abandonTrial)
addQuestTrigger(SYS_ON_DIE, abandonTrial)

setQuestFSMTable(
{
    -- SET [518]
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    -- the [518] branch of @mugong_lightstick, he cannot see why you did not manage it
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '霹雳尊者要考验你，跟他说一声就可以进训练场。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '进训练场',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是很奇怪嘛</par>
                            <par>凭你的能力那里面好像没有你制服不了的怪物。。</par>
                            <par>无论如何，再试一次吗？</par>
                            <par></par>
                            <par><event id="npc_go_trial">好的，拜托您了。</event></par>
                            <par><event id="npc_explain">训练场里要做什么？</event></par>
                            <par><event id="npc_not_yet">累积些经验，请以后再来！</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_lightstick_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想修炼雷电术，在一定的时间里将训练场内的所有怪物打败即可。</par>
                            <par></par>
                            <par><event id="npc_go_trial">好的，拜托您了。</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_lightstick_next5
                npc_not_yet = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯。。知道了。但是雷电术是魔法师的代表魔法，而且是一定要掌握的魔法。无论如何在最短的时日内掌握雷电术，对你的前途很有帮助。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_lightstick_next4_1
                npc_go_trial = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在要把你送到某一个地方。</par>
                            <par>如果可以将那里<t color="red">所有的怪物打败</t>，就认为你通过了该考验。需要记住的是那里所有的怪物都是可以进行电击魔法的怪物。通过和这种敌人的战斗，提高对电击魔法的理解是这个训练的目的。</par>
                            <par>同时与怪物面对面，你可以掌握谁是首先要攻击的对象。希望你不要做任何不经过思考冲动、无意义的行动。</par>
                            <par>我将你送到那儿的时间是<t color="red">5分钟</t>。。</par>
                            <par>5分钟过去后，你将重新回到这里。那就祝你走运啰！</par>
                            <par></par>
                            <par><event id="npc_enter_trial" close="1">移  动</event></par>
                        </layout>
                    ]=])
                end,

                npc_enter_trial = function(uid, value)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
                end,
            }
        ]])
    end,

    quest_enter_trial = function(uid, args)
        enterTrial(uid)
    end,

    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在训练场里，%d 分钟内打倒僧侣僵尸。别的怪物杀了还会再来。', trialMinutes}
    end,

    -- [519]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '通过了考验，回去找霹雳尊者领雷电术秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领雷电术秘籍',

                -- @mugong_lightstick_complete_next1
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>祝贺你<t color="red">通过</t>了测试！我看你具有修炼雷电术的坚实基础。我已经为你写好了雷电术秘籍，参照练习吧。我再给你一些金币和东西，用在需要的地方。</par>
                            <par>嘿嘿，看见年轻人脸上充满成就感是老年人的最大快乐。修炼武功的过程中还会有困难的，请随时来找我。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '雷电术（秘籍）', 1)
                    server.player.addItem(uid, '闪电眼', 1)
                    server.player.deliverGold(uid, 17000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_lightstick, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- the opening of the 雷电术 description, he gives it whether or not you qualify. the rest
    -- of it only comes with the level
    local blurb = [=[雷电术是在敌人的头上<t color="red">放射雷电，使他们受到惨重破坏的强大魔法</t>。一次只可以攻击一个物体，启动时间有些长，但是破坏力非常大。]=]

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼雷电术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [753] 1. the legacy line asks the question inverted, kept as written
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有收到雷电术秘籍吗？哦，年轻人<t color="red">健忘症</t>也太严重了嘛。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkjob wizard
            if not server.player.hasJob(uid, '法师') then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>该武功不是其它职业的人很容易就熟练的武功，只有<t color="red">魔法师</t>可以掌握。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 雷电术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你好像已经修炼了<t color="red">雷电术</t>。。。如果这样就没有向我接受训练的必要了。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_lightstick_next2, checklevel 16
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>%s</par>
                        <par>但是凭你现在的实力好像还不能学习雷电术。经过一些训练后，再来吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], blurb, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>%s 如果没有抵抗闪电的能力，而受到该魔法的攻击。。嘿嘿嘿。。。</par>
                    <par>由于雷电术要产生强大的雷电，开始训练的时候比学习任何魔法都要遇到很大的困难。尤其是开始面向天空发射轻微的闪电，转换为雷的过程是非常困难的。为了熟练掌握该魔法，应提高对<t color="red">电击系列魔法</t>的理解力。</par>
                    <par></par>
                    <par><event id="npc_want_learn">想学习雷电术。</event></par>
                </layout>
            ]=], blurb)
        end,

        -- @mugong_lightstick_next3, SET [518]
        npc_want_learn = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>给你指出修炼雷电术的要点并不难，首先要看你是否具有学习雷电术的<t color="red">资格</t>。如果不这样，学习威力强大魔法时走火入魔的危险会提高。</par>
                    <par>怎么样？接受我的测试吗？</par>
                    <par></par>
                    <par><event id="npc_accept">好的，我要试试。</event></par>
                    <par><event id="npc_not_yet">现在好象还有些勉强。</event></par>
                </layout>
            ]=])

            -- SET [518] lands here, before you have answered, so backing out now still leaves
            -- you on the quest and he greets you with the retry line next time
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,

        -- @mugong_lightstick_next5
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯。。知道了。但是雷电术是魔法师的代表魔法，而且是一定要掌握的魔法。无论如何在最短的时日内掌握雷电术，对你的前途很有帮助。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_lightstick_next4_1
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>现在要把你送到某一个地方。</par>
                    <par>如果可以将那里<t color="red">所有的怪物打败</t>，就认为你通过了该考验。需要记住的是那里所有的怪物都是可以进行电击魔法的怪物。通过和这种敌人的战斗，提高对电击魔法的理解是这个训练的目的。</par>
                    <par>同时与怪物面对面，你可以掌握谁是首先要攻击的对象。希望你不要做任何不经过思考冲动、无意义的行动。</par>
                    <par>我将你送到那儿的时间是<t color="red">5分钟</t>。。</par>
                    <par>5分钟过去后，你将重新回到这里。那就祝你走运啰！</par>
                    <par></par>
                    <par><event id="npc_enter_trial" close="1">移  动</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightstick_next4_2
        npc_enter_trial = function(uid, value)
            server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
        end,
    })
]])
