-- converted from Envir/QuestDiary/MU_taoist/soulSkel.txt
-- with its kill hook, MonQuest/soulSkel.txt, registered in Envir/MapQuest.txt against
-- 变异骷髅 dying in 试练场_1_014
--
-- 清明子 drops you into 试练场_1_013, a graveyard of eight 变异骷髅 with ten minutes on the
-- clock. seven of them just talk, and three of those take offence at the wrong answer and put
-- 骷髅战将 on you for it. the eighth is the one that matters: talk it round and it will only
-- agree to follow you if you beat it in a duel, which happens next door in 试练场_1_014
--
-- win and it hands over a 幻影玉珠 and comes with you, and 清明子 trades that for the 秘籍
--
-- the eight NPCs map to the legacy scripts by position, from Envir/merchant.txt:
--   变异骷髅_1 44,25 is 15Magic_DoGwan0, the one you duel
--   变异骷髅_2..8 are DoGwan1..7 at 26,20 / 29,23 / 23,23 / 35,15 / 26,26 / 25,41 / 10,24
--
-- flags: [720] done, [504] sent to the graveyard, [505] duel accepted, [506] duel won
--
-- legacy turned a second taoist away with 已经有人在接受测试哟... 请等一下 because everyone
-- shared 1_013 and 1_014. both load as copies here, so nobody qualified is refused

_G.minQuestLevel = 17

_G.magicName = '召唤骷髅'
_G.mijiName  = '召唤骷髅（秘籍）'
_G.giftName  = '幻影玉珠'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

_G.trialMinutes = 10

-- map 1_013, the graveyard, entered at its default spot
_G.yardMap  = '试练场_1_013'
_G.mainSkel = '变异骷髅_1'

-- Param1..3 = 1_014 27 20, then mapmove 1_014 23 24
_G.duelMap = '试练场_1_014'
_G.duelX   = 27
_G.duelY   = 20
_G.startX  = 23
_G.startY  = 24

-- mapmove 1_002 12 11 at the end, and mapmove 1 350 402 if you walk away from the duel
_G.exitMap = '本馆_1_002'
_G.exitX   = 12
_G.exitY   = 11

_G.walkAwayMap = '道馆_1'
_G.walkAwayX   = 350
_G.walkAwayY   = 402

-- the seven that only talk. each is {npc, the lines, and for the three that can be provoked,
-- the rude answer and where the 骷髅战将 turn up}
--
-- skel3 and skel5 roll random 2 on the rude answer, skel7 always punishes it
_G.talkers =
{
    {
        npc   = '变异骷髅_2',
        lines = {'哈哈哈。。像你这种家伙还是看不到我原来的样子。', '在哪儿了。。我要休息的地方。。。'},
        reply = '不知道为什么，好象不是人。',
    },
    {
        npc   = '变异骷髅_3',
        lines = {'啊！别问！什么都别问！'},
        reply = '很奇怪...',
    },
    {
        npc   = '变异骷髅_4',
        lines = {'哈哈哈。。像你这种家伙还是看不到我原来的样子。', '长久战斗的日子。但是我们得到的东西什么都没有。。。', '嗯？我正在说什么话？'},
        reply = '好象很长时间一个人了...不幸的灵魂。',
        rude  =
        {
            label   = '哈哈哈，象你长得一样竟说傻话儿...',
            chance  = 2,
            punish  = '愚笨的人，你讲的话使人后悔。',
            spared  = '愚笨的人，要知道今天运气很好。',
            reply   = '什么意义? 这种...',
            spawnAt = {23, 25},
        },
    },
    {
        npc   = '变异骷髅_5',
        lines = {'不要随便进行随机传送。', '没有做好，将成为我现在的样子哟。哈哈哈'},
        reply = '什么话儿?',
    },
    {
        npc   = '变异骷髅_6',
        lines = {'你现在还有带有活人的痕迹，但是马上就会变成我们的样子哟。'},
        reply = '不幸的灵魂啊...别花心思！',
        rude  =
        {
            label   = '别说假话。根本不可能的事儿...',
            chance  = 2,
            punish  = '果真如此吗？哈哈哈。。。',
            spared  = '要知道今天运气很好。',
            reply   = '这种...阴险的家伙。',
            spawnAt = {25, 23},
        },
    },
    {
        npc   = '变异骷髅_7',
        lines = {'我是自豪的远征队的队员！', '这些半兽人，都给我猛扑上。'},
        reply = '说以前是远征队的',
    },
    {
        npc   = '变异骷髅_8',
        lines = {'想回故乡。。。'},
        reply = '快点回家乡吧...',
        rude  =
        {
            label   = '哈哈哈, 忘记了家乡在哪儿？',
            chance  = 1,
            punish  = '唐突的家伙，一点也不考虑别人的处境...',
            reply   = '出现了这种...失误?',
            spawnAt = {10, 24},
        },
    },
}

_G.punishMonster = '骷髅战将'
_G.punishCount   = 2

local function closeTrial(uid)
    local timer = dbGetQuestVar(uid, 'trialTimer')
    if timer then
        dbSetQuestVar(uid, 'trialTimer', nil)
        closeThread(timer)
    end

    for _, key in ipairs({'duelMapUID', 'yardMapUID'}) do
        local mapUID = dbGetQuestVar(uid, key)
        if mapUID then
            dbSetQuestVar(uid, key, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end
end

-- @mugong_recallskel_test_skel1 through skel7, the seven that are only there to talk
local function setupTalkers(uid, yardUID)
    for _, talker in ipairs(talkers) do
        setupInstanceNPCBehavior(yardUID, talker.npc, uid,
        string.format([[ return getUID(), getQuestName(), %s, %s, %s, %s, %d ]],
            asInitString(talker.lines), asInitString(talker.reply),
            talker.rude and asInitString(talker.rude) or 'nil',
            asInitString(punishMonster), punishCount),
        [[
            local questUID, questName, lines, reply, rude, punishMonster, punishCount = ...
            local questPath = {SYS_EPUID, questName}

            local function postLines(uid)
                local parts = {'<layout>'}
                for _, line in ipairs(lines) do
                    table.insert(parts, '<par>' .. line .. '</par>')
                end
                table.insert(parts, '<par></par>')

                if rude then
                    table.insert(parts, string.format('<par><event id="npc_rude">%s</event></par>', rude.label))
                end

                table.insert(parts, string.format('<par><event id="%s" close="1">%s</event></par>', SYS_EXIT, reply))
                table.insert(parts, '</layout>')

                uidPostXML(uid, questPath, table.concat(parts))
            end

            local handlers =
            {
                [SYS_LABEL] = '搭话',
                [SYS_ENTER] = postLines,
            }

            if rude then
                -- the answer it takes badly. skel3 and skel5 let it go half the time, skel7
                -- never does
                handlers.npc_rude = function(uid, value)
                    if (rude.chance > 1) and (math.random(rude.chance) ~= 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>%s</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], rude.spared, SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par></par>
                            <par><event id="%s" close="1">%s</event></par>
                        </layout>
                    ]=], rude.punish, SYS_EXIT, rude.reply)

                    uidRemoteCall(getMapUID(), punishMonster, punishCount, rude.spawnAt[1], rude.spawnAt[2],
                    [=[
                        local name, count, x, y = ...
                        for _ = 1, count do
                            addMonster(name, x, y, false)
                        end
                    ]=])
                end
            end

            return handlers
        ]])
    end
end

-- @mugong_recallskel_test_mainskel, the one that will follow you if you can beat it
local function setupMainSkel(uid, yardUID, retry)
    setupInstanceNPCBehavior(yardUID, mainSkel, uid,
    string.format([[ return getUID(), getQuestName(), %s ]], tostring(retry)),
    [[
        local questUID, questName, retry = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '搭话',

            -- mainskel1, and mainskel_retry1 once it has already taken your challenge
            [SYS_ENTER] = function(uid, value)
                if retry then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>为了战斗又来了。</par>
                            <par>好的，再试一次吗？</par>
                            <par></par>
                            <par><event id="npc_accept_duel">好的，现在就开始。</event></par>
                            <par><event id="npc_refuse_duel">准备好了，再来！</event></par>
                        </layout>
                    ]=])
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>听见喊声了哦。</par>
                        <par>叫我的声音。。。</par>
                        <par>你是谁?</par>
                        <par></par>
                        <par><event id="npc_talk2">为了寻找守护灵而来。</event></par>
                    </layout>
                ]=])
            end,

            -- mainskel2
            npc_talk2 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嘿嘿嘿。。</par>
                        <par>不害怕，找到地牢空间来了。</par>
                        <par>你也有可能成为这个样子，不害怕吗？</par>
                        <par></par>
                        <par><event id="npc_talk3">当然恐惧。</event></par>
                    </layout>
                ]=])
            end,

            -- mainskel3
            npc_talk3 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>很奇怪。。。</par>
                        <par>感受到了<t color="red">命运之手的召唤</t>，我已经不是我了。</par>
                        <par>哦。。好象凭借谁的法力来到这里，在发生更大的事情之前快些离开这里。</par>
                        <par>或者死了，或者成为连死都不行的样子。</par>
                        <par></par>
                        <par><event id="npc_talk4">死一点也不害怕，害怕的是没有实现自己的意愿。</event></par>
                    </layout>
                ]=])
            end,

            -- mainskel4
            npc_talk4 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>...</par>
                        <par>现在知道了。平静的心脏在怦怦地跳动。。。</par>
                        <par>感觉到惊心动魄的兴奋。啊，我希望的东西就在这里。</par>
                        <par>我感觉到了<t color="red">战斗的宿命</t></par>
                        <par>好的，我将按照指示做。</par>
                        <par>但是，有一个<t color="red">条件</t>。</par>
                        <par></par>
                        <par><event id="npc_ask_terms">什么条件?</event></par>
                    </layout>
                ]=])
            end,

            -- mainskel5
            npc_ask_terms = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我是战士。</par>
                        <par>认为名义是最高的价值。。。同时我知道的只有这个。</par>
                        <par>哦，条件很简单。<t color="red">和我搏斗，战胜我，使我屈服。</t></par>
                        <par>如何？ 打吗？</par>
                        <par></par>
                        <par><event id="npc_accept_duel">好的，现在当场开始吧。</event></par>
                        <par><event id="npc_refuse_duel">准备好了，再来！</event></par>
                    </layout>
                ]=])
            end,

            -- mainskel7_1, and 7_2 walks you out of the graveyard entirely
            npc_refuse_duel = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>软弱的人。。。随你的便。</par>
                        <par>我要在这个地方等到何时？</par>
                        <par></par>
                        <par><event id="npc_walk_away" close="1">首先逃出这个地方，重新回到大飞圣僧那儿...</event></par>
                    </layout>
                ]=])
            end,

            npc_walk_away = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_walked_away'})
            end,

            -- mainskel6_1, set [505]
            npc_accept_duel = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>好的。接受<t color="red">你的挑战</t>。</par>
                        <par>那么现在一起去对决场吧。。。</par>
                        <par></par>
                        <par><event id="npc_go_duel" close="1">移动</event></par>
                    </layout>
                ]=])
            end,

            npc_go_duel = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_duel'})
            end,

        }
    ]])
end

-- @mugong_recallskel_next4_3 from Monclear onwards
local function enterYard(uid)
    local yardUID = loadInstanceMap(yardMap)
    if not yardUID then
        server.player.postString(uid, '地牢空间现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    dbSetQuestVar(uid, 'yardMapUID', yardUID)

    setupTalkers(uid, yardUID)
    setupMainSkel(uid, yardUID, dbGetQuestVar(uid, 'duelAccepted') == true)

    -- TimeRecall 10
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialMinutes * 60 * 1000)
        server.player.postString(uid, '时间到了，你被送出了地牢空间。')
        setQuestState{uid = uid, state = 'quest_ready'}
    end))

    -- map 1_013, anywhere on it
    local x, y = uidRemoteCall(yardUID, [[ return getRandLoc() ]])
    server.player.spaceMove(uid, yardUID, x, y)
    setQuestState{uid = uid, state = 'quest_in_yard'}
end

-- MonQuest/soulSkel.txt, the duel won: Monclear 1_014, set [506], map 1_013
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_duel' then
        return
    end

    if getMonsterName(monsterID) ~= '变异骷髅' then
        return
    end

    setQuestState{uid = uid, state = 'quest_duel_beaten'}
end)

local function abandonTrial(uid)
    local state = dbGetQuestState(uid)
    if (state == 'quest_in_yard') or (state == 'quest_in_duel') or (state == 'quest_duel_beaten') then
        setQuestState{uid = uid, state = 'quest_ready'}
    end
end

addQuestTrigger(SYS_ON_ONLINE, abandonTrial)
addQuestTrigger(SYS_ON_OFFLINE, abandonTrial)
addQuestTrigger(SYS_ON_DIE, abandonTrial)

-- the [504] branch of @mugong_recallskel, the offer to go back in
local function setupTeacher(uid)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    [[
        return getUID(), getQuestName()
    ]],
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '再去地牢空间',
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果没有回应召唤的<t color="red">守护灵</t>，即使明白了召唤骷髅的道理也没有任何作用。</par>
                        <par>再次去地牢空间吗？</par>
                        <par></par>
                        <par><event id="npc_retry">好的，去地牢空间。</event></par>
                        <par><event id="npc_explain">地牢空间里要做什么？</event></par>
                        <par><event id="npc_not_yet">准备好了，再来！</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_recallskel_retry1
            npc_retry = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>知道了。</par>
                        <par>精神要集中，希望这次一定可以成功。。。</par>
                        <par></par>
                        <par><event id="npc_enter_yard" close="1">移动</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_recallskel_retry2
            npc_not_yet = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>这种情况下，做好<t color="red">万全的准备</t>还是很明智的。被称为地牢空间的地方是进进出出没有一块儿好地方的场所。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_recallskel_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想学习召唤骷髅，请到地牢空间和将要成为自己<t color="red">守护灵</t>的骷髅直接对决吧。</par>
                        <par>如果对决赢了，这个骷髅将成为你的守护灵。</par>
                        <par></par>
                        <par><event id="npc_enter_yard">好的，去地牢空间。</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_enter_yard = function(uid, value)
                server.quest.setState(questUID, {uid = uid, state = 'quest_enter_yard'})
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
        setQuestDesp{uid=uid, '清明子可以送你去地牢空间，找一个愿意跟着你的守护灵。'}
        setupTeacher(uid)
    end,

    quest_enter_yard = function(uid, args)
        enterYard(uid)
    end,

    quest_in_yard = function(uid, args)
        setQuestDesp{uid=uid, '在地牢空间里找愿意跟你走的骷髅，%d 分钟。', trialMinutes}
    end,

    -- mainskel7_2, out of the graveyard and back to the 道馆
    quest_walked_away = function(uid, args)
        closeTrial(uid)
        server.player.spaceMove(uid, walkAwayMap, walkAwayX, walkAwayY)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    -- mainskel6_2 through 6_4
    quest_enter_duel = function(uid, args)
        -- set [505]
        dbSetQuestVar(uid, 'duelAccepted', true)

        local duelUID = loadInstanceMap(duelMap)
        if not duelUID then
            server.player.postString(uid, '对决场现在进不去，过一会儿再来吧。')
            setQuestState{uid = uid, state = 'quest_in_yard'}
            return
        end

        dbSetQuestVar(uid, 'duelMapUID', duelUID)

        uidRemoteCall(duelUID, duelX, duelY,
        [[
            local x, y = ...
            addMonster('变异骷髅', x, y, false)
        ]])

        server.player.spaceMove(uid, duelUID, startX, startY)
        setQuestState{uid = uid, state = 'quest_in_duel'}
    end,

    quest_in_duel = function(uid, args)
        setQuestDesp{uid=uid, '在对决场里打倒那个变异骷髅。'}
    end,

    -- [506], and win2's map 1_013 puts you back with it to collect
    quest_duel_beaten = function(uid, args)
        setQuestDesp{uid=uid, '赢了决斗，回地牢空间找那个骷髅。'}

        local duelUID = dbGetQuestVar(uid, 'duelMapUID')
        if duelUID then
            dbSetQuestVar(uid, 'duelMapUID', nil)
            closeInstanceMap(duelUID, exitMap, exitX, exitY)
        end

        local yardUID = dbGetQuestVar(uid, 'yardMapUID')
        if yardUID then
            local x, y = uidRemoteCall(yardUID, [[ return getRandLoc() ]])
            server.player.spaceMove(uid, yardUID, x, y)

            -- it has one thing left to say, and only that
            setupInstanceNPCBehavior(yardUID, mainSkel, uid,
            [[
                return getUID(), getQuestName()
            ]],
            [[
                local questUID, questName = ...
                local questPath = {SYS_EPUID, questName}

                return
                {
                    [SYS_LABEL] = '领礼物',
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>金属相碰飞溅的火花，呼呼的喘气声，还有战场上面的血腥味儿。。。但是即使在极限的状况下，我也无法放弃的名义。。。</par>
                                <par>这是给从和我的战斗中取得胜利的你的<t color="red">礼物</t>。谢谢使我想起忘却的东西。<t color="red">现在跟随着你重新回到战场</t>。</par>
                                <par>如果需要的帮助，请随时联系。</par>
                                <par></par>
                                <par><event id="npc_leave_yard">首先要离开这个地方...</event></par>
                            </layout>
                        ]=])

                        server.player.addItem(uid, '幻影玉珠', 1)
                    end,

                    npc_leave_yard = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>(虽然很辛苦, 但是能拥有这么好的伙伴真是很开心啊...)</par>
                                <par></par>
                                <par><event id="npc_done" close="1">结束</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_done = function(uid, value)
                        server.quest.setState(questUID, {uid = uid, state = 'quest_duel_won'})
                    end,
                }
            ]])
        else
            setQuestState{uid = uid, state = 'quest_duel_won'}
        end
    end,

    -- mainskel_complete3, out to 本馆 and back to 清明子
    quest_duel_won = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '守护灵跟上你了，回本馆找清明子领召唤骷髅秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领召唤骷髅秘籍',

                -- @mugong_recallskel_complete1
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，<t color="red">和守护灵的合同</t>成功了？如此这样，现在学习召唤骷髅就没有什么大问题了。。</par>
                            <par>你已经在其它地方得到了武功秘籍，我也没有再给你的必要了。如果可以熟练地掌握这本书，以后即使你一个人修炼没有什么问题。</par>
                            <par>那么更加进步吧！</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '召唤骷髅（秘籍）', 1)
                    server.player.deliverGold(uid, 19000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_recallskel, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- @mugong_recallskel_next2, the same description whether or not you are high enough
    local blurb = [=[召唤骷髅是将在地牢空间里彷徨<t color="red">战士的灵魂召唤回来，一起进行战斗的魔法</t>。由于是召唤回已经死去的人，他们对死亡没有任何的恐惧，只有和敌人战斗的意志，因此可以称为真正的战士。也就是可以称为非常适合一起战斗的同僚。]=]

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼召唤骷髅',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [720] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到召唤骷髅秘籍吗？</par>
                        <par>或者把它丢失在在那儿了？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_recallskel_next1, checkmagic 召唤骷髅
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>感受到保护你的正气了吧!</par>
                        <par>你已经修炼了<t color="red">召唤骷髅</t>，也没有必要在接受我的指教了。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_recallskel_next2, checklevel 17
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>%s你现在修炼的程度还不够，在修炼一下再来！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], blurb, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>%s</par>
                    <par></par>
                    <par><event id="npc_ask_teach">请向我传授召唤骷髅的魔法吧！</event></par>
                </layout>
            ]=], blurb)
        end,

        -- @mugong_recallskel_next3, set [504]
        npc_ask_teach = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>好的，那么你利用守护灵帮助完成见到<t color="red">战士灵魂</t>的合同。但是我能做的就是使你可以遇见战士们的灵魂送你去地牢空间，那以后的事情你要自己看着处理。</par>
                    <par>如果搞不好，就永远无法从地牢空间回来，并且有可能和他们一样成为彷徨的灵魂。</par>
                    <par>喔，怎么办？</par>
                    <par></par>
                    <par><event id="npc_accept">没关系，请送到地牢空间。</event></par>
                    <par><event id="npc_not_yet">准备好了，再来！</event></par>
                </layout>
            ]=])

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,

        -- @mugong_recallskel_next5
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>说实话是很危险的地方，但是只有冒险也才可以得到东西。请做好<t color="red">完备的准备</t>，再来！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_recallskel_next4_1
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么就使你向合适的地牢空间移动。 重复讲一下千万记住那个地方发生的事情你自己都要看着解决。地牢空间是没有任何发表资料的未知空间，时时刻刻要小心。</par>
                    <par>给你的时间是不是<t color="red">10分钟</t>。</par>
                    <par>那么，请安全地回来！</par>
                    <par></par>
                    <par><event id="npc_enter_yard" close="1">移动</event></par>
                </layout>
            ]=])
        end,

        npc_enter_yard = function(uid, value)
            server.quest.setState(questUID, {uid = uid, state = 'quest_enter_yard'})
        end,
    })
]])
