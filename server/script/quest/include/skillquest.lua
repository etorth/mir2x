-- the MU_* skill quests
--
-- a 15Magic teacher will copy any book out for a fee (see npc/include/skillteacher.lua), but
-- for the skills he actually teaches there is a longer road: reach the level, hear him out,
-- and for most of them fetch a material off a particular monster. that road is free
--
-- one quest plugin per skill, all sharing this shape:
--
--     local skillquest = require('quest.include.skillquest')
--
--     skillquest.setSkillQuest
--     {
--         teacher = {'本馆_1_002', '清明子_1'},
--         job     = '道士',
--         level   = 12,
--         book    = '施毒术',                 -- the raw book, consumed at the end
--
--         fetch =                             -- optional, skip for a pure lore quest
--         {
--             item    = '蛆卵',
--             monster = {'毒蜘蛛', '蝎子'},
--             kills   = 5,
--             say     = '（这是蛆卵吗？……）',
--             desp    = '去猎杀毒蜘蛛和蝎子，取得蛆卵。',
--         },
--
--         lore   = {'...', '...'},            -- shown one page at a time before the errand
--         reward = '...',                     -- what he says when handing the 秘籍 over
--     }

local mondrop = require('quest.include.mondrop')

local skillquest = {}

local function manualName(book)
    return book .. '（秘籍）'
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

    local mapName, npcName = args.teacher[1], args.teacher[2]
    assertType(mapName, 'string')
    assertType(npcName, 'string')

    if getItemID(args.book) <= 0 then
        fatalPrintf('Skill quest wants unknown book %s', args.book)
    end

    if getItemID(manualName(args.book)) <= 0 then
        fatalPrintf('No 秘籍 exists for %s', args.book)
    end

    -- the teacher's line while the errand is still open, and the hand-in
    local function teacherBehavior(uid, state)
        setupNPCQuestBehavior(mapName, npcName, uid,
        string.format([[ return getUID(), getQuestName(), %s, %s, %s, %s ]],
            asInitString(args.book),
            asInitString(manualName(args.book)),
            asInitString(args.fetch and args.fetch.item or ''),
            asInitString(args.reward)),
        [[
            local questUID, questName, book, manual, need, reward = ...
            local questPath = {SYS_EPUID, questName}

            local function missing(uid)
                if not server.player.hasItem(uid, book, 1) then
                    return book
                end

                if need ~= '' and not server.player.hasItem(uid, need, 1) then
                    return need
                end
                return nil
            end

            return
            {
                [SYS_LABEL] = '修炼' .. book,
                [SYS_ENTER] = function(uid, args)
                    local lack = missing(uid)
                    if lack then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>还没有把<t color="red">%s</t>带来啊，等你带来了再说吧。</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], lack, SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], reward, SYS_EXIT)

                    server.player.removeItem(uid, book, 1)
                    if need ~= '' then
                        server.player.removeItem(uid, need, 1)
                    end

                    server.player.addItem(uid, manual, 1)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end

    setQuestFSMTable(
    {
        -- errand open, or straight to the hand-in when there is nothing to fetch
        [SYS_ENTER] = function(uid, args2)
            if args.fetch then
                setQuestDesp{uid=uid, args.fetch.desp or ('去取得' .. args.fetch.item .. '，再回去找' .. npcName .. '。')}
            else
                setQuestDesp{uid=uid, '回去找' .. npcName .. '修炼' .. args.book .. '。'}
            end
            teacherBehavior(uid, SYS_ENTER)
        end,
    })

    -- some materials are already in the drop table or sold in a shop, those need no hook
    if args.fetch then
        assertType(args.fetch.item, 'string')
        if args.fetch.monster then
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
    end

    -- the teacher offers it the moment the player is ready for it
    uidRemoteCall(getNPCharUID(mapName, npcName), getUID(), getQuestName(),
        args.job, args.level, args.book, args.lore, args.fetch and args.fetch.item or '',
    [[
        local questUID, questName, job, level, book, lore, need = ...
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

        -- one page per lore line, the last one starts the errand
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

                if need == '' then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par><event id="%s" close="1">知道了</event></par>
                        </layout>
                    ]=], text, SYS_EXIT)
                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>%s</par>
                            <par>先去把<t color="red">%s</t>找来吧。</par>
                            <par><event id="%s" close="1">知道了</event></par>
                        </layout>
                    ]=], text, need, SYS_EXIT)
                end

                server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
            end
        end

        setQuestHandler(questName, handler)
    ]])
end

return skillquest
