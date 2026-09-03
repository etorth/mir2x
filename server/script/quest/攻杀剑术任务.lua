-- converted from Envir/QuestDiary/MU_warrior/wedo.txt
--
-- 龙血先生 does not just hand 攻杀剑术 over. he lends you a 攻杀铁剑, a lump of a sword that
-- can not be taken off once worn (SDItem::EA_BIND), and sends you into a training ground with
-- eleven skeletons and three minutes to clear them
--
-- the ground is a private copy of 试练场_01_003 per attempt, so two warriors never share it —
-- legacy had one shared map and refused you with 有人在接受测试，请等一下 when it was busy
--
-- lose the sword and he will sell you another for 10000. fail and you can retry as often as
-- you like. only on success does he take the sword back and pay out

_G.minQuestLevel = 14

_G.teacherMap = '边境城市_01'
_G.teacherNPC = '龙血先生_1'

_G.trialMap  = '试练场_01_003'
_G.trialNPC  = '龙血先生_1'
_G.trialX    = 9
_G.trialY    = 12

-- where the ground spits you back out, from mapmove 01 456 303
_G.exitMap = '边境城市_01'
_G.exitX   = 456
_G.exitY   = 303

_G.trialSeconds  = 3 * 60
_G.swordPrice    = 10000
_G.trialMonsters = {{'骷髅', 10}, {'骷髅战士', 1}}

local function totalMonsters()
    local count = 0
    for _, entry in ipairs(trialMonsters) do
        count = count + entry[2]
    end
    return count
end

-- hand the lent sword back and clear the copy, used by every way out of the trial
local function leaveTrial(uid, passed)
    local mapUID = dbGetQuestVar(uid, 'trialMapUID')
    if mapUID then
        closeInstanceMap(mapUID, exitMap, exitX, exitY)
        dbSetQuestVar(uid, 'trialMapUID', nil)
    end

    local timer = dbGetQuestVar(uid, 'trialTimer')
    if timer then
        closeThread(timer)
        dbSetQuestVar(uid, 'trialTimer', nil)
    end

    setQuestState{uid = uid, state = passed and 'quest_trial_passed' or 'quest_trial_failed'}
end

-- open a copy of the ground, stock it, drop the player in and start the clock
local function enterTrial(uid)
    local mapUID = loadInstanceMap(trialMap)
    if not mapUID then
        server.player.postString(uid, '训练场现在进不去，稍后再来吧。')
        return false
    end

    for _, entry in ipairs(trialMonsters) do
        uidRemoteCall(mapUID, entry[1], entry[2], trialX, trialY,
        [[
            local monster, count, x, y = ...
            for _ = 1, count do
                addMonster(monster, x, y, false)
            end
        ]])
    end

    dbSetQuestVar(uid, 'trialMapUID', mapUID)

    -- the NPC inside the copy only ever sees this copy's monsters, which is how legacy's
    -- checkmonmap worked
    setupInstanceNPCBehavior(mapUID, trialNPC, uid,
    string.format([[ return getUID(), getQuestName(), %d ]], totalMonsters()),
    [[
        local questUID, questName, total = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '训练场',
            [SYS_ENTER] = function(uid, args)
                -- no argument means every monster on this map, and this map is the copy
                if getMonsterCount() > 0 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请将里面所有的骷髅都处理掉。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>里面所有的骷髅都被处置哟。。好的，有能力。。请在外面看。。</par>
                        <par><event id="npc_leave_trial" close="1">关闭</event></par>
                    </layout>
                ]=])
            end,

            npc_leave_trial = function(uid, args)
                uidRemoteCall(questUID, uid,
                [=[
                    local playerUID = ...
                    _RSVD_NAME_leaveTrial(playerUID, true)
                ]=])
            end,
        }
    ]])

    server.player.spaceMove(uid, trialMap, trialX, trialY)
    server.player.postString(uid, '规定时间是3分钟，抓紧！')

    -- the clock. pause is cancellable, so clearing the timer on the way out stops it
    dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
        pause(trialSeconds * 1000)
        server.player.postString(uid, '时间到了，你被送出了训练场。')
        leaveTrial(uid, false)
    end))

    setQuestState{uid = uid, state = 'quest_in_trial'}
    return true
end

-- reachable from the injected NPC code inside the copy
function _RSVD_NAME_leaveTrial(uid, passed)
    leaveTrial(uid, passed)
end

-- the teacher's side of things, shared by the first run and every retry
local function teacherBehavior(uid, retry)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %d, %s ]], swordPrice, retry and 'true' or 'false'),
    [[
        local questUID, questName, swordPrice, retry = ...
        local questPath = {SYS_EPUID, questName}

        local function wearingSword(uid)
            local item = server.player.getWLItem(uid, WLG_WEAPON)
            return item ~= nil and item.itemID == getItemID('攻杀铁剑')
        end

        return
        {
            [SYS_LABEL] = retry and '再次挑战训练场' or '进入训练场',
            [SYS_ENTER] = function(uid, args)
                if not wearingSword(uid) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>攻杀铁剑如何？只有带着攻杀铁剑来才可以进入训练场。</par>
                            <par><event id="npc_lost_sword">呜呜，攻杀铁剑丢了。</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                if retry then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par><event id="npc_retry_trial">拜托你进行指教。</event></par>
                            <par><event id="npc_not_ready">现在好象有些勉强。</event></par>
                        </layout>
                    ]=])
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>用那种像铁块一样的剑进行实战搏杀，内力都将集中在剑尖儿。希望你可以活着回来，再见面！哈哈哈</par>
                        <par>规定时间是<t color="red">3分钟</t>。。希望你在规定的时间之内可以成功。</par>
                        <par><event id="npc_go_trial">下一步</event></par>
                    </layout>
                ]=])
            end,

            npc_not_ready = function(uid, args)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>警惕轻率虽然是你这个年龄段的人具备困难的美德，但缺乏果断性也不好。不管怎样，如果准备充分了，请随时来接受训练。</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_lost_sword = function(uid, args)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>什么攻杀铁剑丢了？因此不能学习剑术了，是吧。</par>
                        <par>如果这样，请使用我的攻杀铁剑吧。。但是不能就这样给你了。</par>
                        <par>一把剑<t color="red">%d</t>两。。还买吗？</par>
                        <par><event id="npc_buy_sword">即使贵也要买。</event></par>
                        <par><event id="npc_no_money">钱不够，不能买。</event></par>
                    </layout>
                ]=], swordPrice)
            end,

            npc_no_money = function(uid, args)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>没有钱？如果是这样，请找到钱再来。。我等你。</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_buy_sword = function(uid, args)
                if not server.player.removeGold(uid, swordPrice) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你没有钱还说要买攻杀铁剑？如果在说一遍，我就不卖攻杀铁剑了。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>攻杀铁剑在这儿，小心不要丢失了！</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)

                uidRemoteCall(uid,
                [=[
                    addBoundItem(getItemID('攻杀铁剑'))
                ]=])
            end,

            -- the retry gets its own send-off line, @yedo_retry_next_1
            npc_retry_trial = function(uid, args)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嘿嘿。。还是有气派好。那么请将我送到训练场吧。希望你顽强战斗。。</par>
                        <par>规定时间是<t color="red">3分钟</t>。。希望你在规定的时间之内可以成功。</par>
                        <par><event id="npc_go_trial">下一步</event></par>
                    </layout>
                ]=])
            end,

            npc_go_trial = function(uid, args)
                uidRemoteCall(questUID, uid,
                [=[
                    local playerUID = ...
                    _RSVD_NAME_enterTrial(playerUID)
                ]=])
            end,
        }
    ]])
end

function _RSVD_NAME_enterTrial(uid)
    enterTrial(uid)
end

-- 攻杀剑术 is what the whole thing is about, quoted by the teacher whenever he turns you away
local swordLore = '攻杀剑法可以称为放出剑气的入门武功。可以称为为了掌握上乘的武功必须经过的阶段？但是毫无疑问的是很有威力的武功。尤其是可以按照敌人的级别提高命中率和破坏力。'

setQuestFSMTable(
{
    -- carrying the lent sword, the trial is open
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '龙血先生给了你一把攻杀铁剑，戴上它去找他进入训练场。'}
        teacherBehavior(uid, false)
    end,

    -- inside the copy, the teacher outside must not offer another run
    quest_in_trial = function(uid, args)
        setQuestDesp{uid=uid, '在训练场里，三分钟内打倒所有骷髅。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '训练场',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>有人在接受测试，请等一下！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- cleared it, go and collect
    quest_trial_passed = function(uid, args)
        setQuestDesp{uid=uid, '通过了训练场的考验，回去找龙血先生领取攻杀剑术秘籍。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领取攻杀剑术秘籍',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>从你眼睛中放射出的光彩可以看出你已经成功掌握了剑术。祝贺你！你又离高手近了一步。请以后坚持不懈地进行修炼，成为一名真正的侠客。</par>
                            <par><event id="npc_take_reward">下一步</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_reward = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>在这里拿武功秘籍。而且给你一些金币和东西，用在需要的地方。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    -- the sword was only ever a loan, and EA_BIND means this is the one way off
                    uidRemoteCall(uid,
                    [=[
                        removeWearItem(WLG_WEAPON)
                    ]=])

                    server.player.addItem(uid, '攻杀剑术（秘籍）', 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 19000)
                    server.player.addItem(uid, '黑珍珠戒指', 1)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,

    -- ran out of time or walked out, he shrugs and lets you try again
    quest_trial_failed = function(uid, args)
        setQuestDesp{uid=uid, '训练场的考验失败了，戴好攻杀铁剑再去找龙血先生重试。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '训练场的结果',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你现在还年轻不要向失败低头，请再试试。树砍了十遍，没有不倒的。如果不放弃，成功的日子终究要来临的。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        -- and the retry entry sits alongside it
        teacherBehavior(uid, true)
    end,
})

uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, swordLore,
[[
    local questUID, questName, minQuestLevel, swordLore = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if server.quest.getState(questUID, {uid=uid}) ~= nil then
                return false
            end
            return server.player.hasJob(uid, '战士')
        end,

        [SYS_ENTER] = function(uid, args)
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>%s</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], swordLore, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>几乎是从现在开始修炼武功的年轻人，我在你这个年纪的时候也经历了同样的问题。。</par>
                    <par>按照那种理由我要帮助你的训练。虽然有些困难。。。打算怎么办？</par>
                    <par><event id="npc_accept">无论如何请传授方法。</event></par>
                    <par><event id="npc_explain">这个训练是怎么进行的？</event></par>
                    <par><event id="npc_hesitate">好象有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_yedo_explain, the rules laid out before you commit
        npc_explain = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>为了学习攻杀剑术，带上我给的<t color="red">攻杀铁剑</t>后，在一定的时间里，请将训练场内的所有怪兽都打倒。</par>
                    <par>攻杀铁剑特点上，佩戴上一次就不会自己脱落。但是在昏迷或者失去耐久性的时候，就可以摘下来。</par>
                    <par>为了通过测试一定要佩戴攻杀铁剑，如果丢失了，请花钱买！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_hesitate = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你个人认为自己的功力还很不足，但是我认为你已经充分具备了学习攻杀剑法的能力，没有必要如此谦虚。不要犹豫，请在尽早的时间里拿出勇气，挑战看看！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept = function(uid, args)
            -- legacy checkmagic, nothing to teach someone who already has it
            if server.player.hasMagic(uid, '攻杀剑术') then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经掌握攻杀剑术吗？</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>攻杀剑法可以说是习剑的入门武功，他的要领可以说是剑术的修炼。当然不是现在就要求通过消耗功力，而攻击到武器不能到达地方的上乘剑术。现在为了研习攻杀剑术只要精神集中修炼就行。即要经历剑和魂合一的阶段才行。只有通过这样的修炼，才能练成对敌的急所发出强力一击的厚实剑术。进入修炼之前，首先要装备此剑。但一旦此剑被抓在手中，直到攻杀剑术修练厚实前，不能脱手，要铭记此点。</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            uidRemoteCall(uid,
            [=[
                addBoundItem(getItemID('攻杀铁剑'))
            ]=])

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
