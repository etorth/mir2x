-- the MU_* skill quests
--
-- a 15Magic teacher will copy any book out for a fee (npc/include/skillteacher.lua), but for
-- the skills he teaches himself there is a longer road, and it is free. every one of them is
-- built from the same handful of steps:
--
--   level gate, and he turns away anyone who already knows the skill (legacy checkmagic)
--   a page or two of lore
--   optionally a material to fetch off a monster
--   optionally a weapon he lends you that can not be taken off (SDItem::EA_BIND), and will
--     sell you again if you lose it
--   optionally a timed trial in a private copy of a 试练场 map, clear it or be thrown out
--   the 秘籍, plus gold and a keepsake
--
--     local skillquest = require('quest.include.skillquest')
--
--     skillquest.setSkillQuest
--     {
--         teacher = {'本馆_1_002', '清明子_1'},
--         job     = '道士',
--         level   = 12,
--         book    = '施毒术',
--
--         fetch = {item = '蛆卵', monster = {'洞蛆'}, kills = 5, say = '（……）'},
--         lend  = {item = '焱火剑', price = 5000},
--
--         trial =
--         {
--             map      = '试练场_1_008',
--             x = 5, y = 14,
--             minutes  = 5,
--             monsters = {{'毒蜘蛛', 3}, {'蝎子', 3}},
--             exit     = {'本馆_1_002', 11, 11},
--         },
--
--         lore   = {'...'},
--         reward = '...',
--         gold   = 14000,
--         items  = {'天仙之珠'},
--         genderItems = {male = '神奇灵魂战衣（男）', female = '神奇灵魂战衣（女）'},
--     }

local mondrop = require('quest.include.mondrop')

local skillquest = {}

local function manualName(book)
    return book .. '（秘籍）'
end

local function asItemList(arg)
    if arg == nil then
        return {}
    end
    return (type(arg) == 'string') and {arg} or arg
end

function skillquest.setSkillQuest(args)
    assertType(args, 'table')
    assertType(args.teacher, 'table')
    assertType(args.job, 'string')
    assertType(args.level, 'integer')
    assertType(args.book, 'string')
    assertType(args.lore, 'table')
    assertType(args.reward, 'string')

    assertType(args.fetch, 'table', 'nil')
    assertType(args.lend, 'table', 'nil')
    assertType(args.trial, 'table', 'nil')
    assertType(args.gold, 'integer', 'nil')
    assertType(args.genderItems, 'table', 'nil')

    local mapName, npcName = args.teacher[1], args.teacher[2]
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    -- gsub returns a count too, keep it out of the format calls below
    local teacherName = (npcName:gsub('_1$', ''))
    local rewardItems = asItemList(args.items)

    for _, name in ipairs({args.book, manualName(args.book), table.unpack(rewardItems)}) do
        if getItemID(name) <= 0 then
            fatalPrintf('Skill quest refers to unknown item %s', name)
        end
    end

    if args.lend and getItemID(args.lend.item) <= 0 then
        fatalPrintf('Skill quest lends unknown item %s', args.lend.item)
    end

    -- what the teacher still wants before he will start, nil when nothing is missing
    local function lacking(uid)
        if args.fetch and not server.player.hasItem(uid, args.fetch.item, 1) then
            return args.fetch.item
        end
        return nil
    end

    -- pay out and finish
    local function payReward(uid)
        server.player.addItem(uid, manualName(args.book), 1)

        if args.gold then
            server.player.addItem(uid, SYS_GOLDNAME, args.gold)
        end

        for _, name in ipairs(rewardItems) do
            server.player.addItem(uid, name, 1)
        end

        if args.genderItems then
            -- getGender is true for male
            local name = server.player.getGender(uid) and args.genderItems.male or args.genderItems.female
            if name and getItemID(name) > 0 then
                server.player.addItem(uid, name, 1)
            end
        end

        if args.fetch then
            server.player.removeItem(uid, args.fetch.item, 1)
        end

        -- the lent weapon was only ever a loan, EA_BIND leaves this the one way off
        if args.lend then
            uidRemoteCall(uid, [[ removeWearItem(WLG_WEAPON) ]])
        end
    end

    ------------------------------------------------------------------ the trial

    local trial = args.trial

    local function leaveTrial(uid, passed)
        local mapUID = dbGetQuestVar(uid, 'trialMapUID')
        if mapUID then
            closeInstanceMap(mapUID, trial.exit[1], trial.exit[2], trial.exit[3])
            dbSetQuestVar(uid, 'trialMapUID', nil)
        end

        local timer = dbGetQuestVar(uid, 'trialTimer')
        if timer then
            closeThread(timer)
            dbSetQuestVar(uid, 'trialTimer', nil)
        end

        setQuestState{uid = uid, state = passed and 'quest_trial_passed' or 'quest_trial_failed'}
    end

    local function enterTrial(uid)
        local mapUID = loadInstanceMap(trial.map)
        if not mapUID then
            server.player.postString(uid, '训练场现在进不去，稍后再来吧。')
            return
        end

        for _, entry in ipairs(trial.monsters) do
            uidRemoteCall(mapUID, entry[1], entry[2], trial.x, trial.y,
            [[
                local monster, count, x, y = ...
                for _ = 1, count do
                    addMonster(monster, x, y, false)
                end
            ]])
        end

        dbSetQuestVar(uid, 'trialMapUID', mapUID)

        -- the NPC inside the copy only ever sees this copy's monsters, which is what legacy's
        -- checkmonmap did
        setupInstanceNPCBehavior(mapUID, npcName, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '训练场',
                [SYS_ENTER] = function(uid, args)
                    if getMonsterCount() > 0 then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>请将里面所有的怪物都处理掉。</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>里面所有的怪物都被处置哟。。好的，有能力。。请在外面看。。</par>
                            <par><event id="npc_leave_trial" close="1">关闭</event></par>
                        </layout>
                    ]=])
                end,

                npc_leave_trial = function(uid, args)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        _RSVD_NAME_skillTrialLeave(playerUID, true)
                    ]=])
                end,
            }
        ]])

        server.player.spaceMove(uid, trial.map, trial.x, trial.y)
        server.player.postString(uid, string.format('规定时间是%d分钟，抓紧！', trial.minutes))

        -- the clock. pause is cancellable, so clearing it on the way out stops it firing late
        dbSetQuestVar(uid, 'trialTimer', runQuestThread(function()
            pause(trial.minutes * 60 * 1000)
            server.player.postString(uid, '时间到了，你被送出了训练场。')
            leaveTrial(uid, false)
        end))

        setQuestState{uid = uid, state = 'quest_in_trial'}
    end

    -- the injected NPC code inside the copy calls back through these
    _G._RSVD_NAME_skillTrialLeave = leaveTrial
    _G._RSVD_NAME_skillTrialEnter = enterTrial

    ------------------------------------------------------- the teacher outside

    -- retry is the same exchange with a different send-off, legacy split them the same way
    local function teacherBehavior(uid, retry)
        setupNPCQuestBehavior(mapName, npcName, uid,
        string.format([[ return getUID(), getQuestName(), %s, %s, %s, %s, %s, %s ]],
            asInitString(args.book),
            asInitString(args.lend and args.lend.item or ''),
            tostring(args.lend and args.lend.price or 0),
            asInitString(args.reward),
            tostring(trial ~= nil),
            tostring(retry)),
        [[
            local questUID, questName, book, lendItem, lendPrice, reward, hasTrial, retry = ...
            local questPath = {SYS_EPUID, questName}

            local function wearingLend(uid)
                if lendItem == '' then
                    return true
                end

                local item = server.player.getWLItem(uid, WLG_WEAPON)
                return item ~= nil and item.itemID == getItemID(lendItem)
            end

            local handler =
            {
                [SYS_LABEL] = retry and ('再次挑战' .. book) or ('修炼' .. book),
                [SYS_ENTER] = function(uid, args)
                    if not wearingLend(uid) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par><t color="red">%s</t>如何？只有带着它来才可以进入训练场。</par>
                                <par><event id="npc_lost_lend">呜呜，%s丢了。</event></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], lendItem, lendItem, SYS_EXIT)
                        return
                    end

                    if not hasTrial then
                        -- nothing to prove, he just wants the material and hands it over
                        uidRemoteCall(questUID, uid,
                        [=[
                            local playerUID = ...
                            _RSVD_NAME_skillHandIn(playerUID, questPath)
                        ]=])
                        return
                    end

                    if retry then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>嘿嘿。。还是有气派好。那么请将我送到训练场吧。希望你顽强战斗。。</par>
                                <par><event id="npc_go_trial">下一步</event></par>
                                <par><event id="npc_not_ready">现在好象有些勉强。</event></par>
                            </layout>
                        ]=])
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>希望你可以活着回来，再见面！哈哈哈</par>
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

                npc_lost_lend = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>什么%s丢了？因此不能学习了，是吧。</par>
                            <par>如果这样，请使用我的吧。。但是不能就这样给你了。</par>
                            <par>一把<t color="red">%d</t>两。。还买吗？</par>
                            <par><event id="npc_buy_lend">即使贵也要买。</event></par>
                            <par><event id="npc_no_money">钱不够，不能买。</event></par>
                        </layout>
                    ]=], lendItem, lendPrice)
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

                npc_buy_lend = function(uid, args)
                    if not server.player.removeGold(uid, lendPrice) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你没有钱还说要买？如果在说一遍，我就不卖了。</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s在这儿，小心不要丢失了！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], lendItem, SYS_EXIT)

                    uidRemoteCall(uid, lendItem,
                    [=[
                        local name = ...
                        addBoundItem(getItemID(name))
                    ]=])
                end,

                npc_go_trial = function(uid, args)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        _RSVD_NAME_skillTrialEnter(playerUID)
                    ]=])
                end,
            }
            return handler
        ]])
    end

    -- the hand-in, used by the no-trial quests and by the passed state
    local function handIn(uid)
        local lack = lacking(uid)
        if lack then
            server.player.postString(uid, string.format('还没有把%s带来。', lack))
            return
        end

        payReward(uid)
        setQuestState{uid = uid, state = SYS_DONE}
    end

    _G._RSVD_NAME_skillHandIn = function(uid)
        handIn(uid)
    end

    local function handInBehavior(uid)
        setupNPCQuestBehavior(mapName, npcName, uid,
        string.format([[ return getUID(), getQuestName(), %s, %s ]], asInitString(args.book), asInitString(args.reward)),
        [[
            local questUID, questName, book, reward = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领取' .. book .. '秘籍',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], reward, SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        _RSVD_NAME_skillHandIn(playerUID)
                    ]=])
                end,
            }
        ]])
    end

    ------------------------------------------------------------------ the FSM

    local fsm =
    {
        [SYS_ENTER] = function(uid, args2)
            if trial then
                setQuestDesp{uid=uid, string.format('去找%s进入训练场，修炼%s。', teacherName, args.book)}
                teacherBehavior(uid, false)
            elseif args.fetch then
                setQuestDesp{uid=uid, args.fetch.desp or ('去取得' .. args.fetch.item .. '，再回去找' .. teacherName .. '。')}
                handInBehavior(uid)
            else
                setQuestDesp{uid=uid, string.format('回去找%s修炼%s。', teacherName, args.book)}
                handInBehavior(uid)
            end
        end,
    }

    if trial then
        assertType(trial.map, 'string')
        assertType(trial.monsters, 'table')
        assertType(trial.exit, 'table')
        assertType(trial.minutes, 'integer')

        for _, entry in ipairs(trial.monsters) do
            if getMonsterID(entry[1]) <= 0 then
                fatalPrintf('Trial spawns unknown monster %s', entry[1])
            end
        end

        fsm['quest_in_trial'] = function(uid, args2)
            setQuestDesp{uid=uid, string.format('在训练场里，%d分钟内打倒所有怪物。', trial.minutes)}

            setupNPCQuestBehavior(mapName, npcName, uid,
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
        end

        fsm['quest_trial_passed'] = function(uid, args2)
            setQuestDesp{uid=uid, string.format('通过了训练场的考验，回去找%s领取%s秘籍。', teacherName, args.book)}
            handInBehavior(uid)
        end

        fsm['quest_trial_failed'] = function(uid, args2)
            setQuestDesp{uid=uid, string.format('训练场的考验失败了，再去找%s重试。', teacherName)}
            teacherBehavior(uid, true)
        end
    end

    setQuestFSMTable(fsm)

    if args.fetch and args.fetch.monster then
        mondrop.setDropOnKill
        {
            {
                monster = args.fetch.monster,
                state   = SYS_ENTER,
                kills   = args.fetch.kills or 1,
                once    = true,
                give    = args.fetch.item,
                say     = args.fetch.say,
            },
        }
    end

    ------------------------------------------------------- the teacher's offer

    uidRemoteCall(getNPCharUID(mapName, npcName), getUID(), getQuestName(),
        args.job, args.level, args.book, args.lore,
        args.fetch and args.fetch.item or '',
        args.lend and args.lend.item or '',
    [[
        local questUID, questName, job, level, book, lore, need, lendItem = ...
        local questPath = {SYS_EPQST, questName}

        local handler =
        {
            [SYS_CHECKACTIVE] = function(uid)
                if server.quest.getState(questUID, {uid=uid}) ~= nil then
                    return false
                end
                return server.player.hasJob(uid, job)
            end,

            [SYS_ENTER] = function(uid, args)
                if server.player.getLevel(uid) < level then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想熟练%s，武功级别最少要达<t color="red">%d</t>级以上。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], book, level, SYS_EXIT)
                    return
                end

                -- legacy checkmagic, nothing to teach someone who already has it
                if server.player.hasMagic(uid, book) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你不是已经掌握%s吗？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], book, SYS_EXIT)
                    return
                end

                if not server.player.hasItem(uid, book, 1) then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>有了<t color="red">%s</t>魔法书，我可以教你魔法。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], book, SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>想学习%s的样子。练习武功的过程中将遇到各种困难，我将给你进行详细地说明。</par>
                        <par>那么，在给你武功秘籍之前，先对武功进行简单的说明吗？</par>
                        <par><event id="npc_lore_1">拜托了！</event></par>
                        <par><event id="%s" close="1">没有必要</event></par>
                    </layout>
                ]=], book, SYS_EXIT)
            end,
        }

        -- one page per lore line, the last one starts the quest
        for i, text in ipairs(lore) do
            handler['npc_lore_' .. i] = function(uid, args)
                if i < #lore then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par><event id="%s">继续</event></par>
                        </layout>
                    ]=], text, 'npc_lore_' .. (i + 1))
                    return
                end

                local tail = ''
                if need ~= '' then
                    tail = string.format('先去把%s找来吧。', need)
                elseif lendItem ~= '' then
                    tail = string.format('进入修炼之前，首先要装备%s。但一旦被抓在手中，直到修练厚实前，不能脱手，要铭记此点。', lendItem)
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>%s</par>
                        <par>%s</par>
                        <par><event id="%s" close="1">知道了</event></par>
                    </layout>
                ]=], text, tail, SYS_EXIT)

                if lendItem ~= '' then
                    uidRemoteCall(uid, lendItem,
                    [=[
                        local name = ...
                        addBoundItem(getItemID(name))
                    ]=])
                end

                server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
            end
        end

        setQuestHandler(questName, handler)
    ]])
end

return skillquest
