-- converted from Envir/QuestDiary/MU_wizard/fireStorm.txt
--
-- not a fight. 霹雳尊者 drops you in the far corner of 试练场_02_002 and waits at the far
-- corner of 试练场_02_003, and you have five minutes to reach him by running the whole
-- diagonal past forty monsters on each map without killing a single one of them
--
-- the pass check is checkmonmap 02_002 40 and checkmonmap 02_003 40, and forty is exactly what
-- gets spawned on each, so one kill anywhere fails the run: 出现失误了。即使仅仅一头怪兽被杀死，
-- 其它怪兽也都全不行了。
--
-- this is the only trial that spans two maps. both are loaded as copies and the gate grids on
-- the 02_002 copy get a trigger that refuses the mapSwitchList destination — that would land
-- you on the shared base 02_003 — and moves you into this run's own copy instead
--
-- flags: [749] done, [500] run started, [501] run passed

_G.minQuestLevel = 12

_G.magicName = '抗拒火环'
_G.mijiName  = '抗拒火环（秘籍）'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

_G.trialMinutes = 5

-- mapmove 02_002 21 85, the corner you start in
_G.firstMap = '试练场_02_002'
_G.firstX   = 21
_G.firstY   = 85

-- the mapSwitchList destination of the 02_002 gate, kept so the copies line up with the base
_G.secondMap = '试练场_02_003'
_G.secondX   = 22
_G.secondY   = 83

-- he stands here, GLOC of 试练场_02_003.霹雳尊者_1
_G.trialNPC = '霹雳尊者_1'

-- mapmove 02 266 146
_G.exitMap = '银杏山谷_02'
_G.exitX   = 266
_G.exitY   = 146

-- the gate out of 02_002, from its mapSwitchList
_G.gateGrids =
{
    {90, 15, 1, 2},
    {91, 14, 1, 3},
    {92, 13, 1, 3},
}

-- the three clusters on each map, Param1..3 then MonGen. forty a map either way, which is what
-- the pass check counts
_G.firstSpawns =
{
    {40, 64, {{'半兽人', 10}, {'毒蜘蛛', 10}}},
    {52, 52, {{'半兽战士', 5}}},
    {65, 40, {{'半兽勇士', 10}, {'掷斧骷髅', 5}}},
}

_G.secondSpawns =
{
    {40, 64, {{'半兽人', 5}, {'毒蜘蛛', 15}}},
    {52, 52, {{'半兽战士', 5}}},
    {65, 40, {{'半兽勇士', 5}, {'掷斧骷髅', 10}}},
}

local function countSpawns(spawnList)
    local total = 0
    for _, cluster in ipairs(spawnList) do
        for _, entry in ipairs(cluster[3]) do
            total = total + entry[2]
        end
    end
    return total
end

local function stockMap(mapUID, spawnList)
    for _, cluster in ipairs(spawnList) do
        uidRemoteCall(mapUID, cluster[1], cluster[2], cluster[3],
        [[
            local x, y, entryList = ...
            for _, entry in ipairs(entryList) do
                for _ = 1, entry[2] do
                    addMonster(entry[1], x, y, false)
                end
            end
        ]])
    end
end

local function closeTrial(uid)
    local timer = dbGetQuestVar(uid, 'trialTimer')
    if timer then
        dbSetQuestVar(uid, 'trialTimer', nil)
        closeThread(timer)
    end

    -- second first, the player is more likely to be standing on it
    for _, key in ipairs({'secondMapUID', 'firstMapUID'}) do
        local mapUID = dbGetQuestVar(uid, key)
        if mapUID then
            dbSetQuestVar(uid, key, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end
end

-- @mugong_firewind_next5_2 through next8
local function enterTrial(uid)
    local firstUID  = loadInstanceMap(firstMap)
    local secondUID = firstUID and loadInstanceMap(secondMap)

    if not (firstUID and secondUID) then
        if firstUID then
            closeInstanceMap(firstUID, exitMap, exitX, exitY)
        end
        server.player.postString(uid, '考场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    stockMap(firstUID, firstSpawns)
    stockMap(secondUID, secondSpawns)

    dbSetQuestVar(uid, 'firstMapUID', firstUID)
    dbSetQuestVar(uid, 'secondMapUID', secondUID)

    -- the gate. left to itself it would send the player to the base 02_003, so refuse it and
    -- hand them to this run's copy
    for _, grid in ipairs(gateGrids) do
        for dy = 0, grid[4] - 1 do
            setupInstanceGridTrigger(firstUID, grid[1], grid[2] + dy, uid,
            string.format([[ return %d, %d, %d ]], secondUID, secondX, secondY),
            [[
                local secondUID, x, y = ...
                return function(uid, gridX, gridY)
                    server.player.spaceMove(uid, secondUID, x, y)
                    return false
                end
            ]])
        end
    end

    -- @mugong_firewind_test, he counts what is still breathing on both maps
    setupInstanceNPCBehavior(secondUID, trialNPC, uid,
    string.format([[ return getUID(), getQuestName(), %d, %d, %d ]], firstUID, countSpawns(firstSpawns), countSpawns(secondSpawns)),
    [[
        local questUID, questName, firstUID, firstTotal, secondTotal = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '考场',
            [SYS_ENTER] = function(uid, value)
                local firstLeft = uidRemoteCall(firstUID, [=[ return getMonsterCount() ]=])
                local secondLeft = uidRemoteCall(getMapUID(), [=[ return getMonsterCount() ]=])

                -- @mugong_firewind_test_fail
                if (firstLeft < firstTotal) or (secondLeft < secondTotal) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>出现失误了。即使仅仅一头怪兽被杀死，其它怪兽也都全不行了。</par>
                            <par>需要将来再次挑战了。。。</par>
                            <par></par>
                            <par><event id="npc_fail_trial" close="1">结束</event></par>
                        </layout>
                    ]=])
                    return
                end

                -- @mugong_firewind_test_pass1
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>恭喜你，干得很好。请首先离开这个地方。</par>
                        <par></par>
                        <par><event id="npc_pass_trial" close="1">结束</event></par>
                    </layout>
                ]=])
            end,

            npc_fail_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_ready'})
            end,

            -- SET [501]
            npc_pass_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_trial_passed'})
            end,
        }
    ]])

    -- TimeRecall 5
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了考场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.spaceMove(uid, firstUID, firstX, firstY)
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

setQuestFSMTable(
{
    -- set [500]
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    -- [500] and not [501], @mugong_firewind_next4_1 offers the run again
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '霹雳尊者答应教你抗拒火环，跟他说一声就可以进考场。'}

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
                [SYS_ENTER] = function(uid, value)
                    -- checkmagic 抗拒火环, he notices if you picked it up in the meantime
                    if server.player.hasMagic(uid, '抗拒火环') then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>那么辛苦学会的抗拒火环，不知道能否很好地使用。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么，请送去吧！</par>
                            <par>我给你送到那儿的时间是<t color="red">5分钟</t>。。时间结束后，你将重新回到这里。</par>
                            <par></par>
                            <par><event id="npc_enter_trial">移  动</event></par>
                            <par><event id="npc_explain">考场里要做什么？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_firewind_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想学习抗拒火环，只有在一定的时间内<t color="red">顺利通过</t>考场才可以。</par>
                            <par>我将站在终点，你将重新回到这里。需要注意的是<t color="red">不能伤害考场内的任何一头怪物</t></par>
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
        setQuestDesp{uid=uid, '穿过考场找到另一头的霹雳尊者，%d 分钟内，而且一头怪物都不能杀。', trialMinutes}
    end,

    -- [501]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '通过了考场，回去找霹雳尊者领抗拒火环秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领抗拒火环秘籍',

                -- @mugong_firewind_give
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>恭喜你！干得好！</par>
                            <par>这里有可以掌握抗拒火环的武功书（秘籍）。。好好使用吧。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '抗拒火环（秘籍）', 1)
                    server.player.deliverGold(uid, 12000)
                    server.player.addItem(uid, '风之黑檀项链', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_firewind, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼抗拒火环',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [749] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书了吗？那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想知道叫“抗拒火环“的武功吗？</par>
                    <par>抗拒火环是一种被敌人包围时，在自己周围产生<t color="red">强烈的火墙</t>，从而逃脱包围的魔法。也是体力弱魔术师必须掌握的魔法。</par>
                    <par>但是仅凭语言是无法理解的，只用直接被敌人包围，并体验生命受到威胁才可以学会的。但是这种方法太粗糙。。。要试一下吗？</par>
                    <par></par>
                    <par><event id="npc_ask_teach">拜托指教了</event></par>
                    <par><event id="npc_not_yet">现在好像有些勉强</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_firewind_next1_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不要自满虽然很重要，但是该果敢的时候就要果敢。如果你的想法如此，我也不干涉。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_firewind_next1_1, checklevel 12, then next2's checkmagic
        npc_ask_teach = function(uid, value)
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嗯。。想学习的想法值得表扬，但修炼的程度好像还不够。修炼一下再来吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经掌握了抗拒火环，请回去吧！我很忙。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_firewind_next3, set [500]
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>知道了。那么告诉你方法。现在我把你送到怪物出没的地方。</par>
                    <par>我站在房间的另一侧，无论有任何事情<t color="red">都不能干扰怪物或者杀死怪物，只能向我跑过来</t>。</par>
                    <par>时间只有<t color="red">5分钟</t>。如果准备好了，请说一下！</par>
                    <par></par>
                    <par><event id="npc_accept">准备好了</event></par>
                    <par><event id="npc_wait">等一下，现在。。。</event></par>
                </layout>
            ]=])

            -- set [500] lands here, before you have answered, so backing out now still leaves
            -- you on the quest and he offers the run again next time
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,

        -- @mugong_firewind_next4_2
        npc_wait = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>比看起来软弱。。。如果确实准备好了，再来吧！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_firewind_next4_1
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么，请送去吧！</par>
                    <par>我给你送到那儿的时间是<t color="red">5分钟</t>。。时间结束后，你将重新回到这里。</par>
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
