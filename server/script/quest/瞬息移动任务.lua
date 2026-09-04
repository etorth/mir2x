-- converted from Envir/QuestDiary/MU_wizard/teleport.txt
-- with its ten hooks, MonQuest/QMteleport1L.txt through QMteleport5R.txt, registered in
-- Envir/MapQuest.txt against the two 沙漠树魔 on each of 试练场_02_010 through 02_014
--
-- a maze of five forks. each of the five maps has a 沙漠树魔 blocking the left way out and
-- another blocking the right, and killing one is how you pick that direction and move on. after
-- the fifth choice the whole path is judged at once: get it right and you are out, get it wrong
-- and the forks are wiped and restocked and you start over, with ten minutes for all of it
--
-- 霹雳尊者 will give you one of three hints about which way to go, and they are riddles about
-- how the choices relate rather than the directions themselves
--
-- the two blockers are 沙漠树魔61 and 沙漠树魔62 in legacy, 61 on the right and 62 on the left.
-- mir2x has no numbered variants, so 沙漠树魔 is the right-hand one here and 沙漠树魔0 the
-- left — the same monster to look at, and the names are what the hooks tell apart
--
-- four things in the legacy data that do not work, all of them left as they are:
--
--   @mugong_fly_check3 wants [507] and [508] both set, and those are the two directions of the
--   same fork, so it can never match. only two of the three paths are really valid
--
--   @mugong_fly_next4_3's ELSEACT goes to @mugong_fly_next4_3, itself
--
--   @mugong_fly_next4_4 is a fourth hint that nothing ever reaches, and it carries the
--   如此没有运气吗？ line for when no hint comes out. that line is used here
--
--   the level branch offers <好像有些勉强。/@mugong_fly_next2_2> but the label is next1_2
--
-- flags: [751] done, [504] sent to the maze, [505]..[514] two per fork, [515] path was right

_G.minQuestLevel = 14

_G.magicName = '瞬息移动'
_G.mijiName  = '瞬息移动（秘籍）'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

_G.trialMinutes = 10

-- the blockers, and which way each of them is the way to
_G.rightMonster = '沙漠树魔'
_G.leftMonster  = '沙漠树魔0'

_G.rightAt = {55, 91}
_G.leftAt  = {94, 44}

-- mapmove 02_0NN 17 8, the same corner on every fork
_G.forkX = 17
_G.forkY = 8

-- Mapmove 02 265 145
_G.exitMap = '银杏山谷_02'
_G.exitX   = 265
_G.exitY   = 145

-- the five forks and what else is standing around on each of them, from @mugong_fly_next8_1
-- onwards. fork five doubles up on the blockers
_G.forks =
{
    {
        map      = '试练场_02_010',
        blockers = 1,
        filler   = {{17, 51, {{'骷髅精灵', 4}}}, {58, 8, {{'掷斧骷髅', 2}}}, {36, 29, {{'骷髅战士', 2}}}},
    },
    {
        map      = '试练场_02_011',
        blockers = 1,
        filler   = {{17, 51, {{'沃玛勇士', 4}}}, {58, 8, {{'沃玛勇士', 4}}}, {36, 29, {{'火焰沃玛', 2}}}},
    },
    {
        map      = '试练场_02_012',
        blockers = 1,
        filler   = {{17, 51, {{'掷斧骷髅', 4}}}, {58, 8, {{'骷髅精灵', 4}}}, {36, 29, {{'骷髅战士', 3}}}},
    },
    {
        map      = '试练场_02_013',
        blockers = 1,
        filler   = {{17, 51, {{'沃玛勇士', 4}}}, {58, 8, {{'沃玛勇士', 4}}}, {36, 29, {{'火焰沃玛', 3}}}},
    },
    {
        map      = '试练场_02_014',
        blockers = 2,
        filler   = {{17, 51, {{'骷髅精灵', 4}}}, {58, 8, {{'掷斧骷髅', 3}}}, {36, 29, {{'骷髅战士', 3}}}},
    },
}

-- what it says as you take each fork, from @MapQuest_move_1to2L onwards
_G.pickLines =
{
    {'(第一次选择左侧。...)', '(第一次选择右侧。。)'},
    {'(第二次选择左侧。。)', '(第二次选择右侧...)'},
    {'(第三次选择左侧。。)', '(第三次选择右侧。。)'},
    {'(第四次选择左侧。)', '(第四次选择右侧。。)'},
    {'（最后第五次选择 左侧。。。现在有2头怪兽拦着路）', '（最后第五次选择 右侧。。。现在有2头怪兽拦着路）'},
}

-- @MapQuest_move_5to0L and 5to0R, for killing the other blocker after you already chose
_G.alreadyChose = {'（既然已经选择了左边... 就往那儿走吧。）', '（既然选择了右边... 就往那儿走吧。）'}

-- @mugong_fly_check1 and check2. check3 can not match, see the note at the top
_G.goodPaths =
{
    {'L', 'L', 'R', 'L', 'R'},
    {'R', 'L', 'L', 'R', 'R'},
}

-- @mugong_fly_next4_1 through next4_4, one riddle each. legacy rolls random 3 down the chain
-- and gives up if none of them hits
_G.hints =
{
    {
        '在第一个岔道口要选择的方向和在第三个岔道口要选择的方向相反',
        '在第二个岔道口要选择的方向和在第四个岔道口要选择的方向相同',
        '在第三个岔道口要选择的方向和在第五个岔道口要选择的方向相同',
        '在第五个岔道口要选择的方向和在第二个岔道口要选择的方向相反',
    },
    {
        '在第一个岔道口要选择的方向和在第三个岔道口要选择的方向相反',
        '在第二个岔道口要选择的方向和在第四个岔道口要选择的方向相反',
        '在第三个岔道口要选择的方向和在第五个岔道口要选择的方向相反',
        '在第五个岔道口要选择的方向和在第二个岔道口要选择的方向相反',
    },
    {
        '在第一个岔道口要选择的方向和在第三个岔道口要选择的方向相同',
        '在第二个岔道口要选择的方向和在第四个岔道口要选择的方向相同',
        '在第三个岔道口要选择的方向和在第五个岔道口要选择的方向相反',
        '在第五个岔道口要选择的方向和在第二个岔道口要选择的方向相同',
    },
}

local function stockFork(mapUID, fork)
    uidRemoteCall(mapUID, rightMonster, fork.blockers, rightAt[1], rightAt[2],
    [[
        local name, count, x, y = ...
        for _ = 1, count do
            addMonster(name, x, y, false)
        end
    ]])

    uidRemoteCall(mapUID, leftMonster, fork.blockers, leftAt[1], leftAt[2],
    [[
        local name, count, x, y = ...
        for _ = 1, count do
            addMonster(name, x, y, false)
        end
    ]])

    for _, cluster in ipairs(fork.filler) do
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

local function forkUID(uid, index)
    return dbGetQuestVar(uid, 'forkMapUID' .. index)
end

local function closeTrial(uid)
    local timer = dbGetQuestVar(uid, 'trialTimer')
    if timer then
        dbSetQuestVar(uid, 'trialTimer', nil)
        closeThread(timer)
    end

    for index = #forks, 1, -1 do
        local mapUID = forkUID(uid, index)
        if mapUID then
            dbSetQuestVar(uid, 'forkMapUID' .. index, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end

    dbSetQuestVar(uid, 'forkPath', nil)
end

-- @mugong_fly_restart0 onwards: wipe every fork and put them back the way they were
local function restockForks(uid)
    dbSetQuestVar(uid, 'forkPath', nil)

    for index, fork in ipairs(forks) do
        local mapUID = forkUID(uid, index)
        if mapUID then
            uidRemoteCall(mapUID, [[ clearMonster() ]])
            stockFork(mapUID, fork)
        end
    end
end

-- @mugong_fly_next7 through next13
local function enterTrial(uid)
    local uidList = {}

    for index, fork in ipairs(forks) do
        local mapUID = loadInstanceMap(fork.map)
        if not mapUID then
            for _, openUID in ipairs(uidList) do
                closeInstanceMap(openUID, exitMap, exitX, exitY)
            end
            server.player.postString(uid, '训练场现在进不去，过一会儿再来吧。')
            setQuestState{uid = uid, state = 'quest_ready'}
            return
        end

        uidList[index] = mapUID
        dbSetQuestVar(uid, 'forkMapUID' .. index, mapUID)
        stockFork(mapUID, fork)
    end

    dbSetQuestVar(uid, 'forkPath', nil)

    -- TimeRecall 10
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了训练场。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    server.player.spaceMove(uid, uidList[1], forkX, forkY)
    setQuestState{uid = uid, state = 'quest_in_trial'}
end

local function pathMatches(path)
    for _, good in ipairs(goodPaths) do
        local same = true
        for index = 1, #good do
            if path[index] ~= good[index] then
                same = false
                break
            end
        end

        if same then
            return true
        end
    end
    return false
end

-- the ten QMteleportNL/NR hooks: which blocker died says which way you went
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_trial' then
        return
    end

    local monsterName = getMonsterName(monsterID)
    local side = (monsterName == leftMonster) and 'L' or ((monsterName == rightMonster) and 'R' or nil)

    if not side then
        return
    end

    local path = dbGetQuestVar(uid, 'forkPath') or {}
    local index = #path + 1

    -- the last fork doubles the blockers, so killing the other kind after you have chosen just
    -- gets you @MapQuest_move_5to0L's ELSESAY
    if index > #forks then
        server.player.postString(uid, alreadyChose[(path[#forks] == 'L') and 1 or 2])
        return
    end

    path[index] = side
    dbSetQuestVar(uid, 'forkPath', path)

    server.player.postString(uid, pickLines[index][(side == 'L') and 1 or 2])

    -- forks one to four just open onto the next one
    if index < #forks then
        local nextUID = forkUID(uid, index + 1)
        if nextUID then
            server.player.spaceMove(uid, nextUID, forkX, forkY)
        end
        return
    end

    -- @MapQuest_fly_check1, the whole path judged at once
    if pathMatches(path) then
        server.player.postString(uid, '（不知道为什么好像可以成功。。嘿嘿）')
        setQuestState{uid = uid, state = 'quest_trial_passed'}
        return
    end

    -- @mugong_fly_failure, reset [505] 9 and put every fork back
    server.player.postString(uid, '（嗯。。。有些混淆，无论如何好像需要重新开始）。')
    restockForks(uid)

    local firstUID = forkUID(uid, 1)
    if firstUID then
        server.player.spaceMove(uid, firstUID, forkX, forkY)
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

-- @mugong_fly_next2 onwards, which is where the [504] branch of @mugong_fly jumps straight to
local function setupTeacher(uid)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %s ]], asInitString(hints)),
    [[
        local questUID, questName, hints = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '进训练场',

            -- @mugong_fly_next2
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>该魔法总是和生死很亲密，即使仔细计算后想超越空间也是几乎不可能的。</par>
                        <par>因此进攻者几乎都是凭借直观力和观察力进行空间超越的。</par>
                        <par></par>
                        <par><event id="npc_how">如何能获得这种能力呢？</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_fly_next2_1
            npc_how = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>正好有适合培养此种能力的场所，到那里去训练吧。到达该场地的过程中，会出现很多岔道。你要不断地同怪物打斗，时间很紧迫，几乎是凭本能选择一个方向走出岔道。</par>
                        <par>如此经过<t color="red">5个岔路口</t>，才可以通过考场。当然是指找到正确出口的情况。在考场通道的最后段要解决掉挡着路的叫<t color="red">沙漠树魔</t>的怪物，然后才可以向<t color="red">下一个考场移动</t>。</par>
                        <par>如果选择了错误的出口。。。你就要重新开始。在规定的时间内，有很多此机会。无论如何，祝你走运。努力试试！</par>
                        <par></par>
                        <par><event id="npc_ready">没有什么问题。</event></par>
                        <par><event id="npc_ask_hint">有些困难，可以给些帮助吗？</event></par>
                        <par><event id="npc_explain">这个测试是怎么进行的？</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_fly_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>为了学习瞬息移动，在考场之内要<t color="red">选择5次岔路口</t>。只有5次都选择正确了，才可以回到我这里。如果错了，又要重新回到起点。训练场之间移动的方法是解决掉挡着路的叫<t color="red">沙漠树魔</t>的怪物，通路中间尽最大可能地回避怪物即可。</par>
                        <par></par>
                        <par><event id="npc_ready">没有什么问题。</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_fly_next3
            npc_ask_hint = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嗯，，，好的。.</par>
                        <par>你要选择的<t color="red">正确通路</t>有<t color="red">3条</t>。。。我给你讲解其中的一种。</par>
                        <par></par>
                        <par><event id="npc_hint">下一步</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_fly_next4_1 through next4_3, each behind its own random 3. if none of
            -- them comes up he gives the 如此没有运气吗？ line from next4_4
            npc_hint = function(uid, value)
                for _, hint in ipairs(hints) do
                    if math.random(3) == 1 then
                        local parts = {'<layout>'}
                        for _, line in ipairs(hint) do
                            table.insert(parts, '<par>' .. line .. '</par>')
                        end

                        table.insert(parts, '<par>记忆好。。5次都要选择正确，发生一次错误都不可以。</par>')
                        table.insert(parts, '<par></par>')
                        table.insert(parts, '<par><event id="npc_ready">下一步</event></par>')
                        table.insert(parts, '</layout>')

                        uidPostXML(uid, questPath, table.concat(parts))
                        return
                    end
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如此没有运气吗？</par>
                        <par>突然想不起来了。一会儿再来。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_fly_next5
            npc_ready = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>现在送到训练场吧。。。</par>
                        <par>我可以将你送到那儿的时间为<t color="red">10分钟</t>。时间结束后，你将重新回到这里。</par>
                        <par>祝你走运。</par>
                        <par></par>
                        <par><event id="npc_enter_trial" close="1">下一步</event></par>
                    </layout>
                ]=])
            end,

            npc_enter_trial = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- set [504]
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '霹雳尊者可以送你去岔路考场，跟他说一声就行。'}
        setupTeacher(uid)
    end,

    quest_enter_trial = function(uid, args)
        enterTrial(uid)
    end,

    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在考场里连过五个岔路口，%d 分钟。走错一次就要从头再来。', trialMinutes}
    end,

    -- [515]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '走对了整条路，回去找霹雳尊者领瞬息移动秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领瞬息移动秘籍',

                -- @mugong_fly_give
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这里有可以掌握瞬息移动的武功书（秘籍）。。好好使用吧。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '瞬息移动（秘籍）', 1)
                    server.player.deliverGold(uid, 19000)
                    server.player.addItem(uid, '变形银蛇戒指', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_fly, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼瞬息移动',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [751] 1. the legacy line asks the question inverted, kept as written
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有收到瞬息移动秘籍吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 14, and below it he explains what the magic is for instead
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>瞬息移动是一种<t color="red">即使没有地牢逃脱卷也可以回到村子附近的魔法</t>。事实上地牢逃脱卷是一种本身带有瞬息移动法力的卷纸。如果连续不断地使用瞬息移动魔法，慢慢地就熟练掌握了该魔法，一次回到村子的概率也将提高。</par>
                        <par>你现在还没有到达修炼的境地，以后再来吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>瞬息移动是一种<t color="red">超越空间而且学习起来非常难的魔法</t>。魔法的发动者计算好自己的位置和将要移动场所间的距离和方位，制造出超越空间的通道。这个过程要在瞬间之内完成，因此是一种学习起来非常复杂的魔法。</par>
                    <par>训练是非常辛苦的。那还要学习瞬息移动吗？</par>
                    <par></par>
                    <par><event id="npc_ask_teach">当然要试试。.</event></par>
                    <par><event id="npc_not_yet">好像有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fly_next1_2, which the option above points at as next2_2 in the legacy text
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不骄傲虽然很重要，但需要果断的时候还是要果断。如果你的想法如此，我也不干涉。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_fly_next1_1, checkmagic 瞬息移动. the legacy line is inverted, kept as written
        npc_ask_teach = function(uid, value)
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你虽然还没有掌握瞬息移动，我很忙请回吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_fly_next2, set [504]. the quest opens at quest_ready and its own behavior
            -- carries that text and everything after it
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>该魔法总是和生死很亲密，即使仔细计算后想超越空间也是几乎不可能的。</par>
                    <par>因此进攻者几乎都是凭借直观力和观察力进行空间超越的。</par>
                    <par></par>
                    <par><event id="%s" close="1">知道了</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
