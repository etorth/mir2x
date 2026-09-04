-- converted from Envir/QuestDiary/MU_wizard/lightShock.txt
--
-- 霹雳尊者 sends you into 试练场_02_001 against fifteen 半兽人 with two minutes to break them.
-- there is a copy of him standing inside, and reporting to him once the room is clear is what
-- ends the trial — the same shape as the warrior ground in 攻杀剑术任务
--
-- flags: [750] done, [502] trial started, [503] trial passed
--
-- legacy turned a second wizard away with 其它魔法师正在接受训练，请等一下 because they all
-- shared the one map. each attempt loads its own copy here, so nobody qualified is refused

_G.minQuestLevel = 13

_G.magicName = '诱惑之光'
_G.mijiName  = '诱惑之光（秘籍）'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

-- Param1..3 = 02_001 9 12, Mongen 半兽人61 15 5, TimeRecall 2
_G.trialMap     = '试练场_02_001'
_G.trialNPC     = '霹雳尊者_1'
_G.trialX       = 9
_G.trialY       = 12
_G.trialCount   = 15
_G.trialMinutes = 2

-- mapmove 02 265 146
_G.exitMap = '银杏山谷_02'
_G.exitX   = 265
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

-- @mugong_lightwave_next5 from Monclear onwards
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '考场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    uidRemoteCall(mapUID, trialCount, trialX, trialY,
    [[
        local count, x, y = ...
        for _ = 1, count do
            addMonster('半兽人', x, y, false)
        end
    ]])

    dbSetQuestVar(uid, 'trialMapUID', mapUID)

    -- @mugong_lightwave_test, the copy of him inside only ever sees this copy's monsters,
    -- which is what legacy's checkmonmap did
    setupInstanceNPCBehavior(mapUID, trialNPC, uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '考场',
            [SYS_ENTER] = function(uid, value)
                if uidRemoteCall(getMapUID(), [=[ return getMonsterCount() ]=]) > 0 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你还没有完全制服半兽人嘛。 剩下的时间不多了。。显示你的威力嘛。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>怪兽们都屈服了。。请在外面看吧！</par>
                        <par></par>
                        <par><event id="npc_leave_trial" close="1">走出考场。</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_lightwave_test_next, SET [503]
            npc_leave_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_trial_passed'})
            end,
        }
    ]])

    -- TimeRecall 2
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了考场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.spaceMove(uid, mapUID, trialX, trialY)
    setQuestState{uid = uid, state = 'quest_in_trial'}
end

-- logging out, dying, or coming back after a restart that took the copy with it all count as
-- walking out of the trial
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
    -- SET [502], he is about to send you in
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_enter_trial'}
    end,

    -- @mugong_lightwave_next6, the offer to go again
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '考场的训练还没通过，再去找银杏山谷的霹雳尊者。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '再进考场',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还要再试一次吗？时间是2分钟，让里面的怪物屈服即可。</par>
                            <par></par>
                            <par><event id="npc_enter_trial">向考场移动。</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
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
        setQuestDesp{uid=uid, '在考场里，%d 分钟内制服所有半兽人，然后找里面的霹雳尊者。', trialMinutes}
    end,

    -- [503], collect from the one standing outside
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '通过了考场的训练，回去找霹雳尊者领诱惑之光秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领诱惑之光秘籍',

                -- @mugong_lightwave_test_give1
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>辛苦了。我知道你可以赢。你本身的威力越强大，上面的怪物就越服从于你。但是要记住怪物的本性是不能被长时间抑制的。也就是说诱惑之光的威力一定时间之后就没有效果了。</par>
                            <par>在这里拿武功书，剩余的部分你要自己学习。。。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '诱惑之光（秘籍）', 1)
                    server.player.deliverGold(uid, 13000)
                    server.player.addItem(uid, '魔家项链', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_lightwave, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼诱惑之光',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [750] 1. the legacy line asks the question inverted, kept as written
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>还没有给你诱惑之光的解析吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 诱惑之光, also inverted in the legacy text
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有掌握称为诱惑之光的魔法吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 13
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有达到修炼诱惑之光的等级。。请继续修炼，达到<t color="red">%d</t>级为止。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            -- checkjob wizard
            if not server.player.hasJob(uid, '法师') then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还不是魔法师吗？如果不是魔法师，还无法修炼该武功。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_lightwave_next3
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想了解诱惑之光？嗯，仅凭单纯的知识是无法修炼诱惑之光的。诱惑之光是使比自己能力低的怪物们<t color="red">精神混乱，从而进行控制的一种魔法</t>。因此为了掌握诱惑之光，要向怪物们显示自己的威力，使他们服从于你的经验是非常重要的。</par>
                    <par></par>
                    <par><event id="npc_accept">拜托您多指教！</event></par>
                    <par><event id="npc_explain">考场里要做什么？</event></par>
                    <par><event id="npc_not_yet">我现在好象有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightwave_explain, plus the blurb sitting in the unreachable ELSESAY of the
        -- [502] check in @mugong_lightwave — it describes the magic and this is the one place
        -- a player can still read it
        npc_explain = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>诱惑之光是一种瞬间里<t color="red">发射威力强大的闪电，使得怪物们恐慌的魔法</t>。如果使用得当，一时间怪物们都无法行动。尤其是可以<t color="red">控制比你能力低很多怪物们精神的可怕魔法</t>。</par>
                    <par></par>
                    <par>如果想学诱惑之光，在一定时间之内将考场内的怪物们都制服即可。</par>
                    <par></par>
                    <par><event id="npc_accept">拜托您多指教！</event></par>
                    <par><event id="npc_not_yet">我现在好象有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightwave_next4_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>跳进去是有些过激的修炼手法。但是又该如何？如果想学习诱惑之光，只有这个方法。。如果做好准备了，请随时来。。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_lightwave_next4_1
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>方法很简单。现在就开始把你送到训练场，与那个地方出现的怪物搏斗，显示你的威力。虽然有些困难，但绝不可以让怪物们看到你软弱的一面。记住一定要让他们知道你是强者的事实。</par>
                    <par>还有限制时间是<t color="red">2分钟</t>。</par>
                    <par></par>
                    <par><event id="npc_enter_trial">向考场移动！</event></par>
                    <par><event id="npc_think_again">仔细想想，再移动！</event></par>
                </layout>
            ]=])
        end,

        -- <仔细想想，再移动！> goes to @mugong_lightwave_next6, which is the retry offer. it
        -- reads the same either way, so he just repeats the terms
        npc_think_again = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>还要再试一次吗？时间是2分钟，让里面的怪物屈服即可。</par>
                    <par></par>
                    <par><event id="npc_enter_trial">向考场移动。</event></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_enter_trial = function(uid, value)
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
