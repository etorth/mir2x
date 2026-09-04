-- converted from Envir/QuestDiary/MU_taoist/holy.txt
-- with its seven hooks, MonQuest/holy1.txt through holy7.txt, registered in Envir/MapQuest.txt
-- against 火焰沃玛 in the 沃玛神殿, the five 困魔咒空间 rooms, and 1_019's guards
--
-- the largest of the skill quests. 大悲善僧 says somebody has been breaking the old 困魔咒
-- wards and sends you to put five of them back. that means five 困魔石 off the 火焰沃玛 in the
-- 沃玛神殿, and then walking the five 困魔咒空间 rooms in order — each gate eats the stone that
-- belongs to it, and the last room has to be cleared out
--
-- legacy hooks all of this on [Enter], a map-entry trigger, and checks every one of the five
-- rooms for another player before letting you in: 好像有谁在里面？有声音... the rooms load as
-- copies here, all five at once, so nobody qualified is refused and the gates between them are
-- grid triggers that keep you inside your own set
--
-- he also warns you not to leave a summoned skeleton standing in a cleansed room. legacy never
-- checks it, so it stays a warning here too
--
-- the door turns away anybody who has no business in the rooms, which is a map-wide grid
-- trigger rather than a per-player one — see setupMapDefaultGridTrigger. a player on the quest
-- has an EPUID trigger on the same grid and EPUID is consulted first, so the two compose
--
-- holy2's 我怎么会在这里呢? 难道我的魂被什么勾住了? and 首先试着离开这个地方 are on its kill
-- hook rather than the door, for somebody killing things in the base 1_019 without the quest.
-- the rooms are empty unless a run stocked them, so there is nothing there to kill and those
-- two lines have no path
--
-- holy.txt also carries a @MapQuest_holycircle_exit saying 暂时到安全的地方 that no MapQuest.txt
-- line calls, so it is dead in the legacy data too
--
-- flags: [726] done, [522] sent for the stones, [517]..[521] one per room entered
--
-- the stones drop with no cap in legacy — holy1 rolls each one in turn and hands over whichever
-- hits, so a duplicate is possible. left that way

_G.minQuestLevel = 27

_G.magicName = '困魔咒'
_G.mijiName  = '困魔咒（秘籍）'

_G.teacherMap = '道馆_1'
_G.teacherNPC = '大悲善僧_1'

-- @MapQuest_holycircle_drop1, rolled in this order and the first hit wins
_G.stoneDrops =
{
    {'第一困魔石', 100, '(这是第一困魔石吗？...要一个不缺地找到5种困魔石...)'},
    {'第二困魔石',  50, '(这是第二困魔石吗？...要一个不缺地找到5种困魔石...)'},
    {'第三困魔石', 100, '(这是第三困魔石吗？...要一个不缺地找到5种困魔石...)'},
    {'第四困魔石',  50, '(这是第四困魔石吗？...要一个不缺地找到5种困魔石...)'},
    {'最后困魔石', 100, '(这是最后困魔石吗？...要一个不缺地找到5种困魔石...)'},
}

_G.stoneMaps =
{
    '沃玛神殿1层_D022',
    '沃玛神殿2层_D023',
    '沃玛神殿1层_D032',
    '沃玛神殿2层_D033',
    '沃玛神殿1层_D042',
    '沃玛神殿2层_D043',
}

-- the way in, from 沃玛神殿2层_D023's mapSwitchList
_G.doorMap   = '沃玛神殿2层_D023'
_G.doorGrids = {{371, 366, 1, 2}, {372, 366, 1, 1}}

-- the five rooms in order, with the stone each gate eats, where that gate is on the map before
-- it, where it puts you, and what waits inside
--
-- gates and spawn points both come straight out of maprecord.inc and holy3.txt
_G.rooms =
{
    {
        map   = '困魔咒空间_1_015',
        stone = '第一困魔石',
        need  = '(如果想进入第一个困魔咒间的入口，要找到第一困魔石哟...)',
        entry = {10, 17},
        spawn = {20, 23, {{'山洞蝙蝠', 5}, {'暗黑战士', 2}}},
        clear = '(这里还没有彻底净化...)',
    },
    {
        map   = '困魔咒空间_1_016',
        stone = '第二困魔石',
        need  = '(需要第二困魔石...)',
        entry = {10, 17},
        spawn = {20, 23, {{'沃玛战士', 3}, {'沃玛勇士', 1}}},
        clear = '(这里还没有彻底净化...)',
    },
    {
        map   = '困魔咒空间_1_017',
        stone = '第三困魔石',
        need  = '(需要第三困魔石...)',
        entry = {10, 17},
        spawn = {20, 23, {{'山洞蝙蝠', 10}, {'沃玛战将', 1}}},
        clear = '(这里还没有彻底净化...)',
    },
    {
        map   = '困魔咒空间_1_018',
        stone = '第四困魔石',
        need  = '(需要第四困魔石...)',
        entry = {10, 17},
        spawn = {20, 23, {{'沃玛勇士', 3}, {'火焰沃玛', 1}}},
        clear = '(现在这个地方还没有清理干净...)',
    },
    {
        map   = '困魔咒空间_1_019',
        stone = '最后困魔石',
        need  = '(需要最后困魔石呢...)',
        entry = {13, 17},
        spawn = {29, 29, {{'沃玛战将', 3}, {'沃玛护卫', 1}}},
    },
}

-- the gate onward, the same three grids on every room but the last
_G.forwardGrids = {{28, 33, 1, 1}, {29, 32, 1, 2}, {30, 32, 1, 1}}

-- and the gate back, which lands you where the room before it starts
_G.backGrids = {{9, 15, 1, 2}}
_G.backGrids19 = {{12, 15, 1, 1}, {13, 16, 1, 1}}
_G.backEntry = {28, 31}

-- 1_019's way out, both to 道馆_1 and to 沃玛神殿2层_D023 in the legacy record. the 道馆
-- destination is the one that matters, it puts you next to 大悲善僧
_G.exitGrids = {{38, 41, 1, 1}, {39, 40, 1, 2}, {40, 40, 1, 1}}
_G.exitMap   = '道馆_1'
_G.exitX     = 350
_G.exitY     = 402

local function eachGrid(gridList, func)
    for _, grid in ipairs(gridList) do
        for dx = 0, grid[3] - 1 do
            for dy = 0, grid[4] - 1 do
                func(grid[1] + dx, grid[2] + dy)
            end
        end
    end
end

local function stockRoom(mapUID, spawn)
    uidRemoteCall(mapUID, spawn[1], spawn[2], spawn[3],
    [[
        local x, y, entryList = ...
        for _, entry in ipairs(entryList) do
            for _ = 1, entry[2] do
                addMonster(entry[1], x, y, false)
            end
        end
    ]])
end

-- the copies live in fld_vars as roomMapUID1..5
local function roomUID(uid, index)
    return dbGetQuestVar(uid, 'roomMapUID' .. index)
end

local function closeRooms(uid)
    for index = #rooms, 1, -1 do
        local mapUID = roomUID(uid, index)
        if mapUID then
            dbSetQuestVar(uid, 'roomMapUID' .. index, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end
end

-- a gate between two copies: the mapSwitchList destination on a copy names the base room, so
-- refuse it and hand the player to the copy this run owns
local function linkRooms(uid, fromUID, toUID, x, y, stone, needLine, clearLine)
    eachGrid(forwardGrids, function(gridX, gridY)
        setupInstanceGridTrigger(fromUID, gridX, gridY, uid,
        string.format([[ return %d, %d, %d, %s, %s, %s ]], toUID, x, y, asInitString(stone), asInitString(needLine), asInitString(clearLine)),
        [[
            local toUID, x, y, stone, needLine, clearLine = ...
            return function(uid, gridX, gridY)
                -- checkmonmap on the room behind you, the ward is not repaired until it is
                -- empty and the next gate will not open on a half-finished one
                if uidRemoteCall(getMapUID(), [=[ return getMonsterCount() ]=]) > 0 then
                    server.player.postString(uid, clearLine)
                    return false
                end

                -- checkitem then take, the gate eats the stone that belongs to it
                if not server.player.hasItem(uid, stone, 1) then
                    server.player.postString(uid, needLine)
                    return false
                end

                server.player.removeItem(uid, stone, 1)
                server.player.spaceMove(uid, toUID, x, y)
                return false
            end
        ]])
    end)
end

-- and the way back, which needs no stone, it just has to stay inside this run's copies
local function linkBack(uid, fromUID, toUID, gridList)
    eachGrid(gridList, function(gridX, gridY)
        setupInstanceGridTrigger(fromUID, gridX, gridY, uid,
        string.format([[ return %d, %d, %d ]], toUID, backEntry[1], backEntry[2]),
        [[
            local toUID, x, y = ...
            return function(uid, gridX, gridY)
                server.player.spaceMove(uid, toUID, x, y)
                return false
            end
        ]])
    end)
end

-- everything @MapQuest_holycircle_moveTo1_1 did, all five rooms stocked in one go
local function enterRooms(uid)
    local uidList = {}

    for index, room in ipairs(rooms) do
        local mapUID = loadInstanceMap(room.map)
        if not mapUID then
            for _, openUID in ipairs(uidList) do
                closeInstanceMap(openUID, exitMap, exitX, exitY)
            end
            server.player.postString(uid, '困魔咒空间现在进不去，过一会儿再来吧。')
            return false
        end

        uidList[index] = mapUID
        dbSetQuestVar(uid, 'roomMapUID' .. index, mapUID)
        stockRoom(mapUID, room.spawn)
    end

    for index = 1, #rooms - 1 do
        linkRooms(uid, uidList[index], uidList[index + 1], rooms[index + 1].entry[1], rooms[index + 1].entry[2], rooms[index + 1].stone, rooms[index + 1].need, rooms[index].clear)
    end

    for index = 2, #rooms do
        linkBack(uid, uidList[index], uidList[index - 1], (index == #rooms) and backGrids19 or backGrids)
    end

    -- 1_019's exit, which closes the whole set down behind you
    eachGrid(exitGrids, function(gridX, gridY)
        setupInstanceGridTrigger(uidList[#rooms], gridX, gridY, uid,
        [[
            return getUID()
        ]],
        [[
            local questUID = ...
            return function(uid, gridX, gridY)
                server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
                return false
            end
        ]])
    end)

    server.player.spaceMove(uid, uidList[1], rooms[1].entry[1], rooms[1].entry[2])
    return true
end

-- holy2, the last room cleared out is the whole thing
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_rooms' then
        return
    end

    local lastUID = roomUID(uid, #rooms)
    if not lastUID then
        return
    end

    -- checkmonmap 1_019 1
    if uidRemoteCall(lastUID, [[ return getMonsterCount() ]]) > 0 then
        server.player.postString(uid, '(这里还没有彻底净化...)')
        return
    end

    server.player.postString(uid, '(终于找到了困魔咒秘籍...)')
    setQuestState{uid = uid, state = 'quest_rooms_done'}
end)

-- logging out or dying in there loses the run, and the stones with it
local function abandonRooms(uid)
    if dbGetQuestState(uid) == 'quest_in_rooms' then
        setQuestState{uid = uid, state = SYS_ENTER}
    end
end

addQuestTrigger(SYS_ON_ONLINE, abandonRooms)
addQuestTrigger(SYS_ON_OFFLINE, abandonRooms)
addQuestTrigger(SYS_ON_DIE, abandonRooms)

-- the [522] branch of @mugong_holycircle, all he says while the wards are still broken
local function setupTeacherNag(uid)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    [[
        return getQuestName()
    ]],
    [[
        local questName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '困魔咒的事',
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>因此没有认为是一件简单的事情，怪兽们的抵抗力是如此的强大。</par>
                        <par>虽然如此也不是就这样可以放弃的事情。</par>
                        <par>困魔咒的房间在<t color="red">沃玛神殿2层里面</t>。</par>
                        <par>迅速将困魔咒复原。</par>
                        <par></par>
                        <par><event id="npc_explain">这件事要怎么做？</event></par>
                        <par><event id="%s" close="1">好的，知道了。</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_holycircle_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想学习困魔咒，首先到沃玛神殿找到<t color="red">五种困魔石</t>。</par>
                        <par>然后从沃玛神殿2层的第1个困魔石房间开始到第5个房间为止，按照顺序处理破坏了困魔石的怪兽们即可。</par>
                        <par>每通过一个房间需要消耗相应的困魔石，如果中间失败了，必须从头开始找到困魔石。</par>
                        <par>处理了最后房间怪兽的头儿，请重新找我来。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- SET [522], and where a lost run drops you back to
    [SYS_ENTER] = function(uid, args)
        closeRooms(uid)
        setQuestDesp{uid=uid, '去沃玛神殿打火焰沃玛，凑齐五种困魔石，再从沃玛神殿2层进困魔咒空间。'}
        setupTeacherNag(uid)

        -- the door in 沃玛神殿2层_D023. it opens on the first stone and takes it
        eachGrid(doorGrids, function(gridX, gridY)
            setupMapGridTrigger(doorMap, gridX, gridY, uid,
            [[
                return getUID()
            ]],
            [[
                local questUID = ...
                return function(uid, gridX, gridY)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_open_rooms'})
                    return false
                end
            ]])
        end)
    end,

    -- standing on the door with the first stone in hand
    quest_open_rooms = function(uid, args)
        if not server.player.hasItem(uid, rooms[1].stone, 1) then
            server.player.postString(uid, rooms[1].need)
            setQuestState{uid = uid, state = SYS_ENTER}
            return
        end

        server.player.removeItem(uid, rooms[1].stone, 1)

        if not enterRooms(uid) then
            setQuestState{uid = uid, state = SYS_ENTER}
            return
        end

        setQuestState{uid = uid, state = 'quest_in_rooms'}
    end,

    quest_in_rooms = function(uid, args)
        setQuestDesp{uid=uid, '在困魔咒空间里，按顺序用困魔石通过五个房间，最后把里面的怪兽全部清掉。'}
    end,

    -- holy2's payout, collected from him rather than handed over on the spot
    quest_rooms_done = function(uid, args)
        closeRooms(uid)
        setQuestDesp{uid=uid, '困魔咒都复原了，回道馆找大悲善僧。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领困魔咒秘籍',

                -- @MapQuest_holycircle_complete_book, and the [726] branch of the main entry
                -- is what he says afterwards
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>托你的福，那个地方的<t color="red">困魔咒被完好地修复了</t>。你去过后加强了那个地方的警卫，以使困魔咒不再受到损伤。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">结束</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '困魔咒（秘籍）', 1)
                    server.player.addItem(uid, '黑除魔戒指', 1)
                    server.player.deliverGold(uid, 28000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @MapQuest_holycircle_moveTo1's [726] / checkmagic / not-[522] branches. the quest's own
-- SYS_ENTER installs an EPUID trigger on the same grids to let its player in, and EPUID wins,
-- so this only ever answers somebody who is not on the quest
for _, grid in ipairs(doorGrids) do
    for dx = 0, grid[3] - 1 do
        for dy = 0, grid[4] - 1 do
            setupMapDefaultGridTrigger(doorMap, grid[1] + dx, grid[2] + dy,
            string.format([[ return getUID(), %s ]], asInitString(magicName)),
            [[
                local questUID, magicName = ...
                return function(uid, x, y)
                    if (server.quest.getState(questUID, {uid = uid}) == SYS_DONE) or server.player.hasMagic(uid, magicName) then
                        server.player.postString(uid, '(现在也没有进去的必要了...)')
                    else
                        server.player.postString(uid, '(现在好像进不去了...)')
                    end
                    return false
                end
            ]])
        end
    end
end

-- holy1, 火焰沃玛 in the 沃玛神殿 while [522] is set. one drop rule per stone, in the order
-- legacy rolls them, and mondrop stops at the first that fires
local mondrop = require('quest.include.mondrop')

local dropList = {}
for _, entry in ipairs(stoneDrops) do
    table.insert(dropList,
    {
        monster = '火焰沃玛',
        map     = stoneMaps,
        state   = SYS_ENTER,
        chance  = entry[2],
        give    = entry[1],
        say     = entry[3],
    })
end

mondrop.setDropOnKill(dropList)

-- @mugong_holycircle, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼困魔咒',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [726] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>托你的福，那个地方的<t color="red">困魔咒被完好地修复了</t>。你去过后加强了那个地方的警卫，以使困魔咒不再受到损伤。</par>
                        <par>现在你也已经掌握了困魔咒吧？你的武功每天突飞猛进地进步，内心也很满足。在不远的将来也许再也没有什么可以教给你了，嘿嘿。。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkjob taoist
            if not server.player.hasJob(uid, '道士') then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>该武功不是其它职业的人们可以掌握的简单武功呀，只有<t color="red">道士</t>才可以掌握。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_holycircle_next1, checkmagic 困魔咒
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经掌握困魔咒嘛？如果到了可以修炼更高水平武功的时候，请重新再来。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_holycircle_next2, checklevel 27. below it he explains the magic in almost
            -- the same words and then refuses to go into where it came from
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>困魔咒是<t color="red">在一定的空间实施魔法，使带有邪气的生物被隔离的技术</t>。带有邪气的生物如果进入困魔咒之内，由于自己体内气体流通不顺而陷入迷惑之中。</par>
                        <par>他们直到受到外部的刺激从魔法中苏醒过来为止，继续在困魔咒中打转转。但是如果有带有正气的人进入，他们将摆脱困魔咒的力量。</par>
                        <par>不要再问更详细的由来，你不是已经到达可以理解该内容的修炼程度。。</par>
                        <par></par>
                        <par><event id="%s" close="1">那么, 以后再来吧！</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>困魔咒是<t color="red">在一定的空间施魔法，使沾有魔气的生物被隔离的魔法</t>。带有邪气的生物如果进入困魔咒之内，会因自身体内气体不顺而陷入迷惑之中。</par>
                    <par>他们直到受到外部的刺激从魔法中苏醒过来为止，不断地在困魔咒中打转转。但是如果带有正气的人进入，他们将摆脱困魔咒的力量。</par>
                    <par>你知道<t color="red">困魔咒的由来</t>吗？</par>
                    <par></par>
                    <par><event id="npc_origin">不知道，请讲。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_holycircle_next3
        npc_origin = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>困魔咒原来是先人为了封闭邪气而创造的古代魔法。过去称为困魔咒的技术和现在的形态有些不同。过去困魔咒的媒体不是护身符，而是叫做<t color="red">困魔石</t>带有新鲜气体的石头。如果使用该种石头，比我们现在称为困魔咒的技术可以在更广泛的区域永久性地压制邪气。</par>
                    <par>现在到处都剩有相同的困魔咒，但是最近这些困魔咒中发生了<t color="red">不一般的事情</t>。</par>
                    <par></par>
                    <par><event id="npc_what_happened">什么不一般的事情?</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_holycircle_next4
        npc_what_happened = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>据某人说<t color="red">困魔咒中有几处被破坏了</t>。现在只能推测是谁故意搞的，但是究竟是谁以什么理由搞的还不是很清楚。现在道馆的很多道士和修炼生正在对此事<t color="red">进行调查或者恢复困魔咒</t>。</par>
                    <par>但是。。。做此事的人手真的很不够，像你一样的有实力者可以成为很大的<t color="red">帮助</t>，你要帮助我们的事情吗？</par>
                    <par></par>
                    <par><event id="npc_accept">好的，我将试试。</event></par>
                    <par><event id="npc_not_yet">我还没有担当此事的能力。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_holycircle_next6
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>现在还有些不相信自己能力的样子。那么做些准备，再来！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_holycircle_next5, SET [522]
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>哈哈哈，我看人还是很有眼力的。</par>
                    <par>你将要担任恢复的困魔咒在<t color="red">沃玛神殿2层的里侧</t>。首先要找到<t color="red">将要恢复的5个困魔祭坛所用的困魔石</t>。从第一困魔石开始到最后一个一共5个困魔石，分散在各处的火焰怪兽有可能握有困魔石。</par>
                    <par>如果5种困魔石都找到了，<t color="red">从第1个困魔咒房间开始按照顺序使用困魔石通过每个房间</t>即可。困魔石在进入需要自己房间的瞬间受到气的感应，将自动修复祭坛。你只要将那个地方<t color="red">破坏祭坛的怪兽都处理掉</t>即可。</par>
                    <par>有一个<t color="red">注意事项</t>，在新鲜的困魔咒房间里<t color="red">不可以召唤自己的白骨</t>。如果召唤，在进入下一个困魔咒房间之前一定要解除召唤。</par>
                    <par>那么请小心身体，快点回来！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
