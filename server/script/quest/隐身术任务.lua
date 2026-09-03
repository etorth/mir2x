-- converted from Envir/QuestDiary/MU_taoist/hiden.txt
--
-- no fight and nothing to collect: 清明子 hides at the far end of two pitch-dark maps and you
-- have five minutes to reach him without a light and without walking into anything. finding
-- him is the whole test, and nothing counts what you killed on the way
--
-- he checks for 蜡烛, 亮蜡烛, 火把 and 亮火把 before he will send you, in your pack and in the
-- torch slot both, and turns you away with a different line depending on which it was
--
-- like 抗拒火环任务 this runs across two maps, 试练场_1_010 into 试练场_1_011. both are loaded
-- as copies and the gate grids on the 1_010 copy get a trigger that refuses the mapSwitchList
-- destination — that would drop you on the shared base 1_011 — and hands you this run's copy
--
-- flags: [721] done, [507] trial started, [508] found him
--
-- legacy turned a second taoist away with 已经有人在接受测试哟... 请等一下 because they all
-- shared the two maps. each attempt loads its own copies here, so nobody qualified is refused

_G.minQuestLevel = 20

_G.magicName = '隐身术'
_G.mijiName  = '隐身术（秘籍）'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

_G.trialMinutes = 5

-- mapmove 1_010 21 85
_G.firstMap = '试练场_1_010'
_G.firstX   = 21
_G.firstY   = 85

-- the mapSwitchList destination of the 1_010 gate
_G.secondMap = '试练场_1_011'
_G.secondX   = 22
_G.secondY   = 83

-- he waits at the far corner, GLOC of 试练场_1_011.清明子_1
_G.trialNPC = '清明子_1'

-- mapmove 1_002 12 11, out of both the trial and the reward
_G.exitMap = '本馆_1_002'
_G.exitX   = 12
_G.exitY   = 11

-- the gate out of 1_010, from its mapSwitchList
_G.gateGrids =
{
    {90, 15, 1, 2},
    {91, 14, 1, 3},
    {92, 13, 1, 3},
}

_G.firstSpawns =
{
    {40, 64, {{'沃玛勇士', 10}, {'雷电僵尸', 10}}},
    {52, 52, {{'沃玛勇士', 5}}},
    {64, 40, {{'火焰沃玛', 5}, {'沃玛护卫', 10}}},
}

_G.secondSpawns =
{
    {40, 64, {{'沃玛勇士', 5}, {'雷电僵尸', 15}}},
    {52, 52, {{'沃玛勇士', 5}}},
    {65, 40, {{'火焰沃玛', 10}, {'沃玛护卫', 5}}},
}

-- what he confiscates, and which word he uses for it
_G.candles  = {'蜡烛', '亮蜡烛'}
_G.torches  = {'火把', '亮火把'}

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

    for _, key in ipairs({'secondMapUID', 'firstMapUID'}) do
        local mapUID = dbGetQuestVar(uid, key)
        if mapUID then
            dbSetQuestVar(uid, key, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end
end

-- @mugong_hiding_next6_3 through next9
local function enterTrial(uid)
    local firstUID  = loadInstanceMap(firstMap)
    local secondUID = firstUID and loadInstanceMap(secondMap)

    if not (firstUID and secondUID) then
        if firstUID then
            closeInstanceMap(firstUID, exitMap, exitX, exitY)
        end
        server.player.postString(uid, '训练场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    stockMap(firstUID, firstSpawns)
    stockMap(secondUID, secondSpawns)

    dbSetQuestVar(uid, 'firstMapUID', firstUID)
    dbSetQuestVar(uid, 'secondMapUID', secondUID)

    for _, grid in ipairs(gateGrids) do
        for dy = 0, grid[4] - 1 do
            setupInstanceGridTrigger(firstUID, grid[1], grid[2] + dy, uid,
            string.format([[ return %d, %d, %d ]], secondUID, secondX, secondY),
            [[
                local secondUID, x, y = ...
                return function(uid, gridX, gridY)
                    server.player.mapUIDMove(uid, secondUID, x, y)
                    return false
                end
            ]])
        end
    end

    -- @mugong_hiding_test. reaching him is all there is to it, he does not look at anything
    setupInstanceNPCBehavior(secondUID, trialNPC, uid,
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
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>祝贺你，做得好！</par>
                        <par>这里危险，请到外面去。</par>
                        <par></par>
                        <par><event id="npc_leave_trial" close="1">结束</event></par>
                    </layout>
                ]=])
            end,

            -- SET [508]
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

    server.player.mapUIDMove(uid, firstUID, firstX, firstY)
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

-- the [507] branch of @mugong_hiding, and @mugong_hiding_next3 for the first run. both end at
-- the same light check and the same 移 动
local function setupTeacher(uid, retry)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %s, %s, %s ]], asInitString(candles), asInitString(torches), tostring(retry)),
    [[
        local questUID, questName, candles, torches, retry = ...
        local questPath = {SYS_EPUID, questName}

        -- @mugong_hiding_next4_1_1 through next4_1_8: pack first, then the torch slot, and the
        -- wording changes on both counts — which item, and where he found it
        local function postLightFound(uid, itemWord, whereWord)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>说了不能使用蜡烛或者火把，现在还拿着呢。</par>
                    <par>请将放在%s的<t color="red">%s</t>放在其它的地方或者%s，然后再来。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], whereWord, itemWord, (whereWord == '书包中' and itemWord == '洋蜡') and '放在地上' or '地上', SYS_EXIT)
        end

        -- returns false when he found something, having said so
        local function checkNoLight(uid)
            for _, name in ipairs(candles) do
                if server.player.hasItem(uid, name, 1) then
                    postLightFound(uid, '洋蜡', '书包中')
                    return false
                end
            end

            for _, name in ipairs(torches) do
                if server.player.hasItem(uid, name, 1) then
                    postLightFound(uid, '火把', '书包中')
                    return false
                end
            end

            -- checkitemw, the torch slot. mir2x wears both candles and torches there
            local worn = server.player.getWLItem(uid, WLG_TORCH)
            if worn then
                for _, name in ipairs(candles) do
                    if worn.itemID == getItemID(name) then
                        postLightFound(uid, '洋蜡', '身上')
                        return false
                    end
                end

                for _, name in ipairs(torches) do
                    if worn.itemID == getItemID(name) then
                        postLightFound(uid, '火把', '身上')
                        return false
                    end
                end
            end
            return true
        end

        -- @mugong_hiding_next5
        local function postReadyToGo(uid)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么，要将你送去了。。</par>
                    <par>我可以将你送到那儿的时间是<t color="red">5分钟</t>。。时间结束后重新回到这里。</par>
                    <par></par>
                    <par><event id="npc_enter_trial" close="1">移 动</event></par>
                </layout>
            ]=])
        end

        return
        {
            [SYS_LABEL] = retry and '再进训练场' or '进训练场',

            [SYS_ENTER] = function(uid, value)
                if retry then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>在如此黑暗之中，好象还没有适应的样子。</par>
                            <par>不要勉强解决问题，保持一个平常心，多试几次终究会成功的。</par>
                            <par>噢噢，现在平静一下心情，想<t color="red">重新挑战一次</t>吗？你这次一定可以成功。</par>
                            <par></par>
                            <par><event id="npc_go_trial">请重新送到训练场！</event></par>
                            <par><event id="npc_explain">训练场里要做什么？</event></par>
                            <par><event id="npc_not_yet">准备好了，再来！</event></par>
                        </layout>
                    ]=])
                    return
                end

                -- @mugong_hiding_next3
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你要在黑暗的<t color="red">训练场里面找到我</t>，注意不要碰到各个地方布置的怪兽。</par>
                        <par>如果被碰上。。。嘿嘿，绝对可以学习到隐藏形迹的方法。好了，现在就送到训练场。无论如何要小心身体。。。</par>
                        <par></par>
                        <par><event id="npc_go_trial">准备好了。</event></par>
                        <par><event id="npc_explain">训练场里要做什么？</event></par>
                        <par><event id="npc_more_prep">还有要准备的事情。</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_hiding_next1_2, only on the retry path
            npc_not_yet = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>说做过什么样的准备，这次也没有特别用处。如果有需要首先解决的事情，解决完再来也可以。也没有急事儿。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_hiding_next4_2
            npc_more_prep = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我不是很忙的人，如果有急事儿解决好再来。任何时候都可以送到训练场。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_hiding_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>为了学习隐身术，在非常黑暗的训练场中不借助<t color="red">洋蜡</t>或者<t color="red">火把</t>的帮助下要找到我。</par>
                        <par>训练场里的怪兽们蠕动着，要尽量回避回到这里。</par>
                        <par></par>
                        <par><event id="npc_go_trial">准备好了。</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_go_trial = function(uid, value)
                if checkNoLight(uid) then
                    postReadyToGo(uid)
                end
            end,

            npc_enter_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- set [507]
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '清明子要考你，跟他说一声就可以进训练场，别带蜡烛和火把。'}
        setupTeacher(uid, false)
    end,

    -- [507] and not [508]
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '黑暗训练场的考验还没通过，再去找本馆的清明子。'}
        setupTeacher(uid, true)
    end,

    quest_enter_trial = function(uid, args)
        enterTrial(uid)
    end,

    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在黑暗的训练场里，%d 分钟内穿过去找到清明子，尽量别碰上怪兽。', trialMinutes}
    end,

    -- [508]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '找到清明子了，回本馆找他领隐身术秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领隐身术秘籍',

                -- @mugong_hiding_give1
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>祝贺你, 你终于成功了！</par>
                            <par>通过在漆黑的空间抢先发现敌人的动静隐藏自己的训练，你的知觉变得很发达了。</par>
                            <par>我给你隐身术秘籍，剩下的部分你自己修炼吧。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '隐身术（秘籍）', 1)
                    server.player.deliverGold(uid, 20000)
                    server.player.addItem(uid, '暗黑凤凰明珠', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_hiding, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼隐身术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [721] 1
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

            -- @mugong_hiding_next1_0
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想知道叫做隐身术的武功吧？</par>
                    <par>隐身术是<t color="red">使怪兽们无法发现自己行踪，从而隐藏自己行踪的魔法</t>。首先不动弹，不被发现。在危急的时候就会有很大的帮助。</par>
                    <par>为了学习隐身术要领会隐藏自己痕迹的方法，因此要到特殊的训练场累积些经验。</par>
                    <par>哦，同时在训练场使用<t color="red">蜡烛</t>或者<t color="red">火把</t>是不可以的，进入训练场的时候都放在地上。如果不这样，我要没收。</par>
                    <par></par>
                    <par><event id="npc_ask_teach">拜托给与指教。</event></par>
                    <par><event id="%s" close="1">准备好了，再来。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_hiding_next1_1, checklevel 20, then next2's checkmagic
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
                        <par>你不是已经掌握隐身术，请到此为止回去吧！我很忙。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_hiding_next3, set [507]. the quest opens there and its own behavior
            -- carries this text and the light check
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你要在黑暗的<t color="red">训练场里面找到我</t>，注意不要碰到各个地方布置的怪兽。</par>
                    <par>如果被碰上。。。嘿嘿，绝对可以学习到隐藏形迹的方法。好了，现在就送到训练场。无论如何要小心身体。。。</par>
                    <par></par>
                    <par><event id="%s" close="1">知道了</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
