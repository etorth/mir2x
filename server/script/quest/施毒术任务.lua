-- converted from Envir/QuestDiary/MU_taoist/amyen.txt
--
-- not a fight either. 清明子 puts you in 试练场_1_008 with four kinds of poisonous thing and
-- five minutes to bring him one of each material off their corpses. a copy of him waits inside
-- and takes the delivery
--
-- 食人花 carries two of the five, everything else one, and mir2x's drop table already has all
-- of them on exactly those monsters (see server/src/dropitemconfig.inc), so nothing extra has
-- to be wired for the materials to appear
--
-- he clears the five materials out of your pack on the way in, so a stock you brought from
-- outside does not count, and again once you have handed them over
--
-- flags: [717] done, [502] trial started, [503] materials handed in
--
-- legacy turned a second taoist away with 有人在接受测试，请等一下！ because they all shared
-- the one map. each attempt loads its own copy here, so nobody qualified is refused

_G.minQuestLevel = 12

_G.magicName = '施毒术'
_G.mijiName  = '施毒术（秘籍）'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

-- Param1..3 = 1_008 5 14, then map 1_008 for the default entry
_G.trialMap     = '试练场_1_008'
_G.trialNPC     = '清明子_1'
_G.trialX       = 5
_G.trialY       = 14
_G.trialMinutes = 5

_G.trialSpawns = {{'毒蜘蛛', 3}, {'食人花', 3}, {'蝎子', 3}, {'洞蛆', 3}}

-- the five he wants one of each of, and the oversized take he clears them with
_G.materials  = {'蛆卵', '蝎子的尾巴', '食人树叶', '食人树的果实', '毒蜘蛛牙齿'}
_G.clearCount = 20

-- mapmove 1_002 11 11
_G.exitMap = '本馆_1_002'
_G.exitX   = 11
_G.exitY   = 11

local function clearMaterials(uid)
    for _, name in ipairs(materials) do
        server.player.removeUpToItem(uid, name, clearCount)
    end
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

-- @mugong_poison_next3 from the takes onwards
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '考场现在进不去，过一会儿再来吧。')
        setQuestState{uid = uid, state = 'quest_ready'}
        return
    end

    -- whatever you walked in with does not count
    clearMaterials(uid)

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

    -- @mugong_poison_test, the copy of him standing in the corner
    setupInstanceNPCBehavior(mapUID, trialNPC, uid,
    string.format([[ return getUID(), getQuestName(), %s, %d ]], asInitString(materials), clearCount),
    [[
        local questUID, questName, materials, clearCount = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '交毒粉材料',
            [SYS_ENTER] = function(uid, value)
                for _, name in ipairs(materials) do
                    if not server.player.hasItem(uid, name, 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>现在材料还没有找齐嘛。我需要的材料是<t color="red">蛆卵 1,蝎子的尾巴 1,食人树叶 1,食人树的果实 1, 毒蜘蛛牙齿  1个</t>。请听好，找到再来。</par>
                                <par>如果在规定的时间里没有找到这些材料，无法修炼施毒术。。请确认材料并告诉我。。没有剩下多少时间了。。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end
                end

                -- @mugong_poison_test_chk_1, one of each goes to him and the rest gets binned
                for _, name in ipairs(materials) do
                    server.player.removeItem(uid, name, 1)
                end

                for _, name in ipairs(materials) do
                    server.player.removeUpToItem(uid, name, clearCount)
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>材料都收集好了哟。。那么出去看吧。。。</par>
                        <par></par>
                        <par><event id="npc_leave_trial" close="1">下一步</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_poison_test_next, set [503]
            npc_leave_trial = function(uid, value)
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

    server.player.mapUIDMove(uid, mapUID, trialX, trialY)
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
    -- SET [502]
    [SYS_ENTER] = function(uid, args)
        setQuestState{uid = uid, state = 'quest_ready'}
    end,

    -- the [502] branch of @mugong_poison, @mugong_poison_next4
    quest_ready = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '清明子要考你采毒粉，跟他说一声就可以进考场。'}

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
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在你还不够水平吗？嗯。。不要失望，请重新挑战。。。</par>
                            <par></par>
                            <par><event id="npc_retry">拜托指教。</event></par>
                            <par><event id="npc_explain">考场里要做什么？</event></par>
                            <par><event id="npc_not_yet">现在好象有些勉强。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_poison_next5_1
                npc_retry = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道了。重新送到考场。请从所有的怪兽那儿采取毒粉。</par>
                            <par></par>
                            <par><event id="npc_enter_trial" close="1">移动到考场。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_poison_next5_2
                npc_not_yet = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>虽然学习、应用施毒术有些复杂，施毒术是道士的<t color="red">唯一进攻辅助魔法</t>，它的效果非常高。</par>
                            <par>现在虽然困难，在最短的时间内掌握施毒术还是要好些。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_poison_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想学习势毒术，处理了训练场的怪兽后，要从他们的尸体上采取<t color="red">蛆卵,蝎子的尾巴,食人树叶,食人树的果实, 毒蜘蛛牙齿</t>。</par>
                            <par>我将站在考场里面，把采取的毒粉交给我。然后再把你重新送到这里。</par>
                            <par></par>
                            <par><event id="npc_enter_trial">移动到考场。</event></par>
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
        setQuestDesp{uid=uid, '在考场里，%d 分钟内采到蛆卵、蝎子的尾巴、食人树叶、食人树的果实、毒蜘蛛牙齿各一个，交给里面的清明子。', trialMinutes}
    end,

    -- [503]
    quest_trial_passed = function(uid, args)
        closeTrial(uid)
        setQuestDesp{uid=uid, '毒粉交齐了，回本馆找清明子领施毒术秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领施毒术秘籍',

                -- @mugong_poison_give
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>辛苦了！给你武功秘籍，剩余的部分自己掌握吧。</par>
                            <par>同时<t color="red">将毒粉放在戴手镯的位置</t>使用即可。虽然开始有些不方便，随着武功级别的增高，也可以和手镯一起戴，不必担心。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '施毒术（秘籍）', 1)
                    server.player.deliverGold(uid, 14000)
                    server.player.addItem(uid, '天仙之珠', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @mugong_poison, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- the opening about poison being a kind of medicine, he gives it twice: once to open with
    -- and again as the whole of what he says if you turn him down
    local intro = [=[知道施毒术。。。毒药实际上是药的一部分并不过分的事实吗？就像为了患者调制药材，毒药也是按照天时和地利使用不同调制方法的复杂东西。如果想学习使用毒药的施毒术，必然需要首先对<t color="red">毒药进行学习</t>。]=]

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼施毒术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [717] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到施毒术秘籍吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 12. the legacy line reads 成为施毒术12级之前，不能学习。, which is
            -- about your own level rather than the magic's, kept as written
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>成为施毒术<t color="red">%d</t>级之前，不能学习。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            -- @mugong_poison_next
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>%s</par>
                    <par></par>
                    <par><event id="npc_ask_teach">拜托指教！</event></par>
                    <par><event id="npc_not_yet">你好象还有些勉强。</event></par>
                </layout>
            ]=], intro)
        end,

        -- @mugong_poison_next2_2, he just says it again
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>%s</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], intro, SYS_EXIT)
        end,

        -- @mugong_poison_next2_1, checkmagic 施毒术 then the briefing
        npc_ask_teach = function(uid, value)
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你已经掌握了施毒术，也没有再学习的必要了。。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>首先对毒粉进行说明。毒粉包括<t color="red">黄色毒粉</t>和<t color="red">灰色毒粉</t>。对这些材料<t color="red">药剂师</t>比我更清楚，请问他们！</par>
                    <par>你在学习施毒术之前，首先要掌握材料的毒性。现在我送你去某个地方，<t color="red">直接采取材料</t>进行学习。采取的方法当作像切肉一样的熟练工种即可。</par>
                    <par>时间是<t color="red">5分钟</t>。。</par>
                    <par></par>
                    <par><event id="npc_enter_trial" close="1">为了掌握毒性而出发。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_poison_next3, SET [502] and set [503] 0
        npc_enter_trial = function(uid, value)
            server.quest.setState(questUID, {uid = uid, state = 'quest_enter_trial'})
        end,
    })
]])
