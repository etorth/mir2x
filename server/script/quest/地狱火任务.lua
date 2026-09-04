-- converted from Envir/QuestDiary/MU_wizard/fireLine.txt
-- with its two hooks, MonQuest/fireLine1.txt and MonQuest/fireLine2.txt, registered in
-- Envir/MapQuest.txt against 试练场_02_005
--
-- 霹雳尊者 hides a book called 新火镜 on one of four 火焰沃玛 in 试练场_02_005 and gives you
-- five minutes to find out which. the other three come back when you kill them, so working
-- through the room is not the way — you want the one carrying it
--
-- two places where the legacy data does not map cleanly, both noted where they bite:
--
--   legacy spawns 火焰沃玛61 three times and 火焰沃玛62 once, and 62 is the one holding the
--   book. 火焰沃玛62 maps to 火焰沃玛, so the marked one is 火焰沃玛 and the other three stay
--   火焰沃玛61. the two records are byte-identical, so this is only a name to tell them apart by
--
--   legacy hooks the pass on [GetItem] 新火镜, so the book hit the ground and you had to grab
--   it while the rest of the room was still on you. the book goes straight into your pack on
--   the kill here and the trial ends there, which is where legacy's pickup ended it too
--
--   @mugong_fireline_complete_next1 does a checkbaggage before paying out and turns you away
--   with 背囊里没有位置了，整理出位置后，请再来！. mir2x has no inventory-full check to hang
--   that on — addInventoryItem takes whatever it is given — so the line has no trigger here
--
-- flags: [756] done, [520] trial started, [521] book in hand

_G.minQuestLevel = 20

_G.magicName = '地狱火'
_G.mijiName  = '地狱火（秘籍）'
_G.bookName  = '新火镜'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

-- Param1..3 = 02_005 15 14, and mapmove 02_005 15 8 drops you in across from them
_G.trialMap     = '试练场_02_005'
_G.trialX       = 15
_G.trialY       = 14
_G.startX       = 15
_G.startY       = 8
_G.trialMinutes = 5

_G.plainMonster  = '火焰沃玛61'
_G.markedMonster = '火焰沃玛'

-- mapmove 02 266 146
_G.exitMap = '银杏山谷_02'
_G.exitX   = 266
_G.exitY   = 146

local function spawnOn(mapUID, name, count)
    uidRemoteCall(mapUID, name, count, trialX, trialY,
    [[
        local name, count, x, y = ...
        for _ = 1, count do
            addMonster(name, x, y, false)
        end
    ]])
end

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

-- @mugong_fireline_next4_2 from Monclear onwards
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '考场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    spawnOn(mapUID, plainMonster, 3)
    spawnOn(mapUID, markedMonster, 1)

    dbSetQuestVar(uid, 'trialMapUID', mapUID)

    -- TimeRecall 5
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了考场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.spaceMove(uid, mapUID, startX, startY)
    setQuestState{uid = uid, state = 'quest_in_trial'}
end

-- fireLine2: the marked one was carrying the book
-- fireLine1: the others come back, one in five times three of them at once
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_trial' then
        return
    end

    local mapUID = dbGetQuestVar(uid, 'trialMapUID')
    if not mapUID then
        return
    end

    local monsterName = getMonsterName(monsterID)

    if monsterName == markedMonster then
        server.player.addItem(uid, bookName, 1)
        server.player.postString(uid, '（嘿，终于通过了学习地狱火的测试。）')
        setQuestState{uid = uid, state = 'quest_trial_passed'}
        return
    end

    if monsterName ~= plainMonster then
        return
    end

    -- random 5
    if math.random(5) == 1 then
        server.player.postString(uid, '哦。。。（这家伙，在瞎说。好像出现了什么失误。。）')
        spawnOn(mapUID, plainMonster, 3)
    else
        server.player.postString(uid, '哦。。。（这家伙，在瞎说。根本没有什么嘛）')
        spawnOn(mapUID, plainMonster, 1)
    end
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
    -- SET [520]
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    -- the [521] branch of @mugong_fireline goes straight back to next4_1
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '霹雳尊者要考你，跟他说一声就可以进考场找新火镜。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '进考场',

                -- @mugong_fireline_next4_1
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>同里面的火焰沃玛打斗时，多少有些麻烦。请注意不要进行近身战。</par>
                            <par>我把你送到那儿的时间是<t color="red">5分钟</t>。。。5分钟过去后，你将重新回到这里。</par>
                            <par>同时你还要牢记<t color="red">新火镜掉落的瞬间，你一定要非常迅速地拿到并回到这个地方。。</t></par>
                            <par></par>
                            <par><event id="npc_enter_trial">移  动</event></par>
                            <par><event id="npc_explain">考场里要做什么？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_fireline_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想学习地狱火，从考场<t color="red">火焰沃玛</t>那儿找到<t color="red">新火镜</t>即可。</par>
                            <par>需要注意的是不拿新火镜的怪物们被打倒之后还要复活，不断地对你进行阻扰。</par>
                            <par>拿到新火镜的一瞬间安全地退回这个地方就可以了。</par>
                            <par></par>
                            <par><event id="npc_enter_trial">移  动</event></par>
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
        setQuestDesp{uid=uid, '在考场里，%d 分钟内找出拿着新火镜的那头火焰沃玛。别的杀了还会再来。', trialMinutes}
    end,

    -- [521]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '拿到新火镜了，回去找霹雳尊者。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交新火镜',

                -- @mugong_fireline_complete
                [SYS_ENTER] = function(uid, value)
                    if not server.player.hasItem(uid, '新火镜', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我把非常辛苦找到的新火镜放在哪儿了？</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>带来了新火镜。那么，好好读一下这个部分吧！</par>
                            <par>嗯。。。如果可以大概理解，可以按照照片摆个姿势吗？</par>
                            <par></par>
                            <par><event id="npc_take_book">这样可以吗？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_fireline_complete_next, and the checkbaggage of next1
                npc_take_book = function(uid, value)
                    if not server.player.hasItem(uid, '新火镜', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我把非常辛苦找到的新火镜放在哪儿了？</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，虽然现在还有些不熟练，但好像已经大体上领悟了新火镜的运气法。这个程度练习地狱火没有任何问题。以后你看书，一个人练习也没有任何问题。</par>
                            <par>你已经在其它地方得到了武功秘籍，我也没有再给你的必要了。我给你一些金币和东西，用在需要的地方。还有你也不在需要新火镜了，我替你保管吧。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '新火镜', 1)
                    server.player.addItem(uid, '地狱火（秘籍）', 1)
                    server.player.addItem(uid, '焰火手镯', 1)
                    server.player.deliverGold(uid, 16000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_fireline, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼地狱火',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [756] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>以前你分明收到了地狱火秘籍吗？你好像记错了。</par>
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
                        <par>该武功不是其它职业的人很容易就可以熟练的武功，只有魔法师可以掌握。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 地狱火
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我看你好像已经掌握了地狱火。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_fireline_next2, checklevel 20
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>地狱火是一种将像波涛一样的熊熊烈火向敌人喷射的魔法。但是你好像还不够学习地狱火的等级。</par>
                        <par>你在训练一下后，再来找我。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>地狱火是一种将<t color="red">像波涛一样的熊熊烈火向敌人喷射的魔法</t>。本魔法的要点是使在进攻者前面展开的烈火沿着一条直线蔓延开，向多数敌人发起进攻时非常有利。将敌人引诱到狭窄的地方沿一列排开，进攻效果最大。</par>
                    <par>好的，凭你的实力是学习地狱火的时候了。但是要想掌握地狱火，应掌握独特的运气法。 叫<t color="red">神火攻</t>的内功新法，按照你的能力坚持不懈练习的话，理解上不会有什么问题的。</par>
                    <par></par>
                    <par><event id="npc_want_learn">想掌握地狱火。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireline_next3, SET [520]
        npc_want_learn = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>好的，如果这样，我在了解你武功实力的同时，还可以给你一个学习地狱火的测试。</par>
                    <par>到我送你去的某地找出叫<t color="red">新火镜</t>的书。当然里面有怪物，其中<t color="red">一个怪物</t>拿着新火镜。你在将怪物打倒，新火镜出现的瞬间马上就抓住书，重新回到现在的地方。请你经常创造机会。</par>
                    <par>怎么样？挑战吗？</par>
                    <par></par>
                    <par><event id="npc_accept">是的，进行挑战。</event></par>
                    <par><event id="npc_not_yet">现在好象有些勉强。</event></par>
                </layout>
            ]=])

            -- SET [520] lands here, before you have answered, so backing out now still leaves
            -- you on the quest and he goes straight to the move offer next time
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,

        -- @mugong_fireline_next5. its <关闭> points at @mugong_fireline_next14_1, a label that
        -- does not exist in the legacy file, so it just closes
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>非常小心的朋友。本臂力尊者像你一样的时候，不知天高地厚, 横冲直撞。不管怎样，好的。你有信心的时候，请随时来！</par>
                    <par></par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_fireline_next4_1
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>同里面的火焰沃玛打斗时，多少有些麻烦。请注意不要进行近身战。</par>
                    <par>我把你送到那儿的时间是<t color="red">5分钟</t>。。。5分钟过去后，你将重新回到这里。</par>
                    <par>同时你还要牢记<t color="red">新火镜掉落的瞬间，你一定要非常迅速地拿到并回到这个地方。。</t></par>
                    <par></par>
                    <par><event id="npc_enter_trial" close="1">移  动</event></par>
                </layout>
            ]=])
        end,

        npc_enter_trial = function(uid, value)
            server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
        end,
    })
]])
