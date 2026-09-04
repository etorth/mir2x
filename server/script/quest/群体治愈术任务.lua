-- converted from Envir/QuestDiary/MU_taoist/massheal.txt
-- with its hooks, MapQuest/massheal.txt and MonQuest/massheal1.txt and massheal2.txt,
-- registered in Envir/MapQuest.txt against 蜈蚣洞穴_1_020 and the 1_023 boss
--
-- the top of the taoist line, and the only skill quest that is a ghost story. 大悲善僧 says he
-- cannot make this year's memorial rite and asks you to go in his place, to a village northeast
-- of 盟重县 by the mouth of 绝命谷. what the villager there asks for has nothing to do with the
-- rite: something in the water source is killing everyone
--
-- kill it and the 秘籍 is simply on you afterwards. 大悲善僧 then explains why: the village died
-- of that plague a hundred years ago and everyone you spoke to was already dead, and the book is
-- what their grudge left behind
--
-- the cave is four maps, 蜈蚣洞穴_1_020 through 1_023, all stocked when you first go in. legacy
-- checks every one of them for another player first — 好像有人在里面？有声音。。。 — and they
-- load as copies here, so nobody qualified is refused
--
-- legacy's 沃毒蜈蚣 and 邪恶蜈蚣 have no mir2x record. 沃毒蜈蚣 maps to 蜈蚣, which is what
-- the boss and the two mid-cave guards are here, and legacy's ordinary 蜈蚣61 maps to 蜈蚣0.
-- the two records are the same monster bar a coolEye flag, so this is only a name to tell them
-- apart by — and since only 1_023 counts, killing a 蜈蚣 anywhere else does nothing
--
-- the two lines the cave mouth gives somebody with no business there — 好像是什么洞窟入口？
-- 现在洞口被堵上了 and 这里是以前击退蜈蚣的洞窟哦 — have no home, the same as 困魔咒任务's:
-- they gate the base cave against every player at once and the quest layer only installs grid
-- triggers per player
--
-- @mugong_massheal_illtown1 gates the villager on daytime night: he only answers after dark and
-- sends you away in daylight. one game day is two real hours, an hour each way, so a wait is
-- never longer than an hour — see isNightTime in serverluamodule.lua
--
-- flags: [730] done, [523] took the charm, [524] the villager asked you, [525] cave stocked,
-- [526] boss dead and the book on you

_G.minQuestLevel = 31

_G.magicName = '群体治愈术'
_G.mijiName  = '群体治愈术（秘籍）'
_G.charmName = '威魂深怨护身符'
_G.charmPrice = 5000

_G.teacherMap = '道馆_1'
_G.teacherNPC = '大悲善僧_1'

_G.villagerMap = '盟重县_74'
_G.villagerNPC = '泼皮路白_1'

-- the trash and the one that matters
_G.trashCentipede = '蜈蚣0'
_G.bossCentipede  = '蜈蚣'

-- the mouth of the cave, from 绝命谷3层_D6004's mapSwitchList
_G.doorMap   = '绝命谷3层_D6004'
_G.doorGrids = {{39, 266, 1, 2}, {40, 267, 1, 1}}

-- and where it puts you, plus the way back out that legacy's mapmove D6004 42 264 uses
_G.caveEntry = {184, 20}
_G.exitMap   = '绝命谷3层_D6004'
_G.exitX     = 41
_G.exitY     = 264

-- the four cave maps in order, what each of them holds, and the gates between them straight
-- out of maprecord.inc
_G.caves =
{
    {
        map   = '蜈蚣洞穴_1_020',
        spawn =
        {
            { 15, 77, {{'跳跳蜂', 10}}},
            { 70, 20, {{'黑色恶蛆', 10}}},
            { 90, 70, {{'跳跳蜂', 10}, {'蜈蚣', 2}}},
            {100, 42, {{'黑色恶蛆', 10}}},
            {146, 18, {{'跳跳蜂', 10}}},
        },
        forward = {{172, 173, 1, 3}},
        nextAt  = {24, 40},
    },
    {
        map   = '蜈蚣洞穴_1_021',
        spawn =
        {
            {25, 30, {{'跳跳蜂', 15}, {'蝴蝶虫', 5}}},
        },
        back    = {{22, 42, 1, 3}, {23, 44, 1, 1}},
        backAt  = {169, 172},
        forward = {{10, 12, 1, 2}, {11, 12, 1, 1}},
        nextAt  = {163, 30},
    },
    {
        map   = '蜈蚣洞穴_1_022',
        spawn =
        {
            { 45, 20, {{'黑色恶蛆', 10}, {'蜈蚣0', 5}}},
            { 65, 70, {{'蜈蚣0', 10}}},
            { 75, 42, {{'黑色恶蛆', 10}, {'蜈蚣', 2}}},
            {122, 17, {{'蜈蚣0', 10}}},
            {154, 30, {{'黑色恶蛆', 10}, {'蜈蚣0', 5}}},
        },
        back    = {{165, 36, 1, 1}, {166, 33, 1, 3}},
        backAt  = {12, 14},
        forward = {{11, 12, 1, 3}, {12, 12, 1, 1}},
        nextAt  = {40, 36},
    },
    {
        map   = '蜈蚣洞穴_1_023',
        spawn =
        {
            {22, 22, {{'蜈蚣', 1}, {'蜈蚣0', 10}}},
        },
        back   = {{41, 40, 1, 1}, {42, 38, 1, 3}},
        backAt = {13, 13},
    },
}

-- 1_020's gate back out to 绝命谷, also from its mapSwitchList
_G.caveExitGrids = {{186, 17, 2, 1}, {187, 18, 1, 1}}

-- massheal2, the trash in the water source comes back two at a time
_G.respawnAt    = {22, 22}
_G.respawnCount = 2

local function eachGrid(gridList, func)
    for _, grid in ipairs(gridList or {}) do
        for dx = 0, grid[3] - 1 do
            for dy = 0, grid[4] - 1 do
                func(grid[1] + dx, grid[2] + dy)
            end
        end
    end
end

local function stockCave(mapUID, spawnList)
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

local function caveUID(uid, index)
    return dbGetQuestVar(uid, 'caveMapUID' .. index)
end

local function closeCaves(uid)
    for index = #caves, 1, -1 do
        local mapUID = caveUID(uid, index)
        if mapUID then
            dbSetQuestVar(uid, 'caveMapUID' .. index, nil)
            closeInstanceMap(mapUID, exitMap, exitX, exitY)
        end
    end
end

-- the gates on a copy still name the base cave, so refuse them and stay inside this run's set
local function linkCaves(uid, fromUID, toUID, gridList, at)
    eachGrid(gridList, function(gridX, gridY)
        setupInstanceGridTrigger(fromUID, gridX, gridY, uid,
        string.format([[ return %d, %d, %d ]], toUID, at[1], at[2]),
        [[
            local toUID, x, y = ...
            return function(uid, gridX, gridY)
                server.player.spaceMove(uid, toUID, x, y)
                return false
            end
        ]])
    end)
end

-- @MapQuest_massheal_cave1_4 from Monclear onwards, set [525]
local function enterCaves(uid)
    local uidList = {}

    for index, cave in ipairs(caves) do
        local mapUID = loadInstanceMap(cave.map)
        if not mapUID then
            for _, openUID in ipairs(uidList) do
                closeInstanceMap(openUID, exitMap, exitX, exitY)
            end
            server.player.postString(uid, '洞窟现在进不去，过一会儿再来吧。')
            return false
        end

        uidList[index] = mapUID
        dbSetQuestVar(uid, 'caveMapUID' .. index, mapUID)
        stockCave(mapUID, cave.spawn)
    end

    for index, cave in ipairs(caves) do
        if cave.forward then
            linkCaves(uid, uidList[index], uidList[index + 1], cave.forward, cave.nextAt)
        end

        if cave.back then
            linkCaves(uid, uidList[index], uidList[index - 1], cave.back, cave.backAt)
        end
    end

    -- 1_020's other gate is the way back out to 绝命谷, which folds the whole cave up
    eachGrid(caveExitGrids, function(gridX, gridY)
        setupInstanceGridTrigger(uidList[1], gridX, gridY, uid,
        [[
            return getUID()
        ]],
        [[
            local questUID = ...
            return function(uid, gridX, gridY)
                server.quest.setState(questUID, {uid = uid, state = 'quest_left_cave'})
                return false
            end
        ]])
    end)

    server.player.spaceMove(uid, uidList[1], caveEntry[1], caveEntry[2])
    return true
end

-- massheal1 and massheal2, both hooked on 1_023
addQuestTrigger(SYS_ON_KILL, function(uid, monsterID)
    if dbGetQuestState(uid) ~= 'quest_in_cave' then
        return
    end

    local lastUID = caveUID(uid, #caves)
    if not lastUID then
        return
    end

    -- only the water source counts, which is what keeps the mid-cave 蜈蚣0 out of it
    if server.player.getMapName(uid) ~= caves[#caves].map then
        return
    end

    local monsterName = getMonsterName(monsterID)

    -- massheal1, the one fouling the water
    if monsterName == bossCentipede then
        server.player.postString(uid, '(几乎都处理哟...)')
        server.player.addItem(uid, mijiName, 1)
        setQuestState{uid = uid, state = 'quest_cave_done'}
        return
    end

    -- massheal2, the rest of them keep coming
    if monsterName == trashCentipede then
        uidRemoteCall(lastUID, trashCentipede, respawnCount, respawnAt[1], respawnAt[2],
        [[
            local name, count, x, y = ...
            for _ = 1, count do
                addMonster(name, x, y, false)
            end
        ]])
    end
end)

local function abandonCave(uid)
    if dbGetQuestState(uid) == 'quest_in_cave' then
        setQuestState{uid = uid, state = 'quest_kill_boss'}
    end
end

addQuestTrigger(SYS_ON_ONLINE, abandonCave)
addQuestTrigger(SYS_ON_OFFLINE, abandonCave)
addQuestTrigger(SYS_ON_DIE, abandonCave)

-- @mugong_massheal_lostCharm1, he will make another for 5000. reachable from both the [523] and
-- [524] branches of @mugong_massheal
local function teacherLostCharmHandlers()
    return
    [[
        npc_lost_charm = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>哈哈，是说将那么重要的威魂深怨护身符丢失了？如果重新制作护身符，要使用非常贵的颜料。那么可以筹备<t color="red">%d</t>两费用吗？</par>
                    <par></par>
                    <par><event id="npc_buy_charm">即使很贵也要重新买到</event></par>
                    <par><event id="npc_no_money">钱不够，无法买。</event></par>
                </layout>
            ]=], charmPrice)
        end,

        -- @mugong_massheal_lostCharm3
        npc_no_money = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么是说钱不够？</par>
                    <par>那么准备好钱，再来！</par>
                    <par>直到等到你找来钱。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_massheal_lostCharm2, checkgold 5000
        npc_buy_charm = function(uid, value)
            if not server.player.removeGold(uid, charmPrice) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你钱都没有，还要威魂深怨护身符？准备好做护身符的材料费，再来 ！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这是威魂深怨护身符。</par>
                    <par>小心不要重新再丢失了。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.player.addItem(uid, '威魂深怨护身符', 1)
        end,
    ]]
end

-- the [523] and [524] branches of @mugong_massheal, both of which point you back out and both
-- of which offer to replace the charm
local function setupTeacherNag(uid, atVillage)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    string.format([[ return getUID(), getQuestName(), %d, %s ]], charmPrice, tostring(atVillage)),
    string.format(
    [[
        local questUID, questName, charmPrice, atVillage = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '祭祖的事',
            [SYS_ENTER] = function(uid, value)
                if atVillage then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>在那个地方该见到谁了嘛。</par>
                            <par>事情都结束了，就回到我这儿吧</par>
                            <par></par>
                            <par><event id="npc_lost_charm">由于失误，弄丢了护身符...</event></par>
                            <par><event id="npc_explain">这件事要怎么做？</event></par>
                            <par><event id="%%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    return
                end

                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>还没有离开那个村庄哟。</par>
                        <par>那个村庄位于<t color="red">盟重县东北方向绝命谷入口的附近</t>。</par>
                        <par>快去快回。</par>
                        <par></par>
                        <par><event id="npc_lost_charm">由于失误，弄丢了护身符...</event></par>
                        <par><event id="npc_explain">这件事要怎么做？</event></par>
                        <par><event id="%%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_massheal_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想学习群体治愈术，带着我给的<t color="red">威魂深怨护身符</t>，去盟重县东北方向绝命谷入口附近的某个村子，上<t color="red">香</t>即可。不知道那个地方将要发生什么事情，剩余的事情你要自己解决。如果所有的问题都解决了，在重新找我来。</par>
                        <par></par>
                        <par><event id="%%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            %s
        }
    ]], teacherLostCharmHandlers()))
end

setQuestFSMTable(
{
    -- SET [523], the charm is yours and the village is waiting
    [SYS_ENTER] = function(uid, args)
        closeCaves(uid)
        setQuestDesp{uid=uid, '带着威魂深怨护身符，去盟重县东北绝命谷入口附近的村子，晚上找那里的人。'}
        setupTeacherNag(uid, false)

        -- @mugong_massheal_illtown, and he only answers at night
        setupNPCQuestBehavior(villagerMap, villagerNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '村子的事',

                -- @mugong_massheal_illtown1, daytime night
                [SYS_ENTER] = function(uid, value)
                    if not isNightTime() then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>现在是大白天。。</par>
                                <par>很刺眼，什么都看不见。。。</par>
                                <par></par>
                                <par><event id="%s" close="1">奇异的人...</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    -- @mugong_massheal_illtown2, checkitem 威魂深怨护身符. without it he has
                    -- nothing to say to you
                    if not server.player.hasItem(uid, '威魂深怨护身符', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>陌生的年青人, 什么事情?</par>
                                <par>晚上的天气很冷，还不快赶路？</par>
                                <par></par>
                                <par><event id="npc_no_charm">奇异的人...</event></par>
                            </layout>
                        ]=])
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>陌生的年青人, 什么事情?</par>
                            <par>年轻的绅士为什么拿着<t color="red">奇怪的护身符</t>走来走去?</par>
                            <par></par>
                            <par><event id="npc_ask_village">有个问题想请教一下。是生活在这个地方的人吗？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_illtown3_2
                npc_no_charm = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>(好像是我要找的村子人，没有任何感兴趣的哟。好像在我身找到什么的样子? 有什么东西落了吗？)</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_massheal_illtown3_1
                npc_ask_village = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果是那样...</par>
                            <par></par>
                            <par><event id="npc_where_village">我受大飞圣僧的委托到村庄来参加祭祀，村庄在哪儿？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_illtown4
                npc_where_village = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>大飞圣僧...?</par>
                            <par>不知道他是谁。 一会儿，请上香。。。</par>
                            <par>啊，这么看来年轻人是武士吗？</par>
                            <par>千万要救救我们吧！</par>
                            <par></par>
                            <par><event id="npc_what_happened">虽然会使用些剑... 到底是什么事情?</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_illtown5
                npc_what_happened = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我们都是生活在<t color="red">百娥村</t>的人。到不久前为止，我们村子还是一个安静适合生活的好地方。但是自从<t color="red">原因不明的传染病</t>开始流行，个把月间不论老幼都吐血而死。</par>
                            <par>村子议员认为生病的原因是水脏。为了确认这个事实，村子里还没有生病的几个人到村子里流淌着的<t color="red">水源所在的洞窟</t>去了。</par>
                            <par>但是只有一个人从那个地方回来了。他满身是疮地回来了，断气之前说<t color="red">蜈蚣</t>们占据了水源，污染了水。</par>
                            <par></par>
                            <par><event id="npc_ask_officials">没有向官吏请求帮助吗？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_illtown6
                npc_ask_officials = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>求了! 请求了!!</par>
                            <par>但是官吏们堵上了流向村子的水流，村子反而被隔离了。我们村子的人们得不到任何帮助，正在死去。</par>
                            <par>我们无法在看人们就这样死去！如果得不到官吏的帮助，即使凭借我们的力量也要除掉蜈蚣们！！因此体格健壮的人们拿着镰刀和镐到蜈蚣所在的洞窟去了。</par>
                            <par>但是仅凭借我们自己的力量无论如何也到达不了水源。千万帮组我们<t color="red">处理那些坏 ??</t>！这样衷肯地拜托你。。。</par>
                            <par></par>
                            <par><event id="npc_accept">知道了，我去那个洞窟看看。</event></par>
                            <par><event id="npc_refuse">非常对不起,也许是非常危险的事情。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_illtown8_1
                npc_refuse = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不行。。。</par>
                            <par>等了很久，又等了很久。。。</par>
                            <par>如果说这是我们的命运，只有寻求其它的<t color="red">救援之手</t>。。。</par>
                            <par>那么请小心走好！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_massheal_illtown7_1, set [524]
                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>谢谢！非常感谢！</par>
                            <par>蜈蚣们栖息在深而且阴森森的叫做绝命的洞窟中。蜈蚣围剿队最后被目击的地方在<t color="red">绝命谷最深地区西南方的某个地方</t>。</par>
                            <par>那个地方正是这个村子水源的所在地，但是现在成了蜈蚣们藏身处的<t color="red">洞窟入口</t>。有可能进到那里边以后就中断了消息。</par>
                            <par></par>
                            <par><event id="npc_what_to_do">在蜈蚣洞窟中要做什么呢？</event></par>
                        </layout>
                    ]=])

                    server.quest.setState(questUID, {uid = uid, state = 'quest_kill_boss'})
                end,

                -- @mugong_massheal_illtown7_2
                npc_what_to_do = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>蜈蚣洞窟非常深，由弯弯曲曲的洞窟连接而成。进到入口后，走很长时间，在中间出现一个宽敞的房间；然后沿着弯弯曲曲的通路走很长时间后就到达了我们这个地方的水源地<t color="red">地下莲池的宽敞空间</t>。</par>
                            <par>首先到那个地方为止，即使有什么事情都要一边小心身体一边前进。因为不知道在中间会遇到什么突变。</par>
                            <par>在水源地有一个污染水源叫做<t color="red">沃毒蜈蚣</t>的家伙，<t color="red">只要把这个家伙处理了就解决了所有的问题</t>。</par>
                            <par></par>
                            <par><event id="%s" close="1">处理了这个家伙就可以了噢。那么我走了</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- [524], the cave door in 绝命谷 will open for you now
    quest_kill_boss = function(uid, args)
        closeCaves(uid)
        setQuestDesp{uid=uid, '去绝命谷最深处西南的蜈蚣洞窟，杀掉污染水源的沃毒蜈蚣。'}
        setupTeacherNag(uid, true)

        -- @mugong_massheal_illtown_retry
        setupNPCQuestBehavior(villagerMap, villagerNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '洞窟的事',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我们在这儿等着勇士回来。千万将污染水源的<t color="red">沃毒蜈蚣</t>处置了。</par>
                            <par>蜈蚣洞窟在<t color="red">绝命谷最深地区西南的某个地方</t>。。。</par>
                            <par></par>
                            <par><event id="%s" close="1">好的，走了。</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        -- @MapQuest_massheal_cave, the mouth of it
        eachGrid(doorGrids, function(gridX, gridY)
            setupMapGridTrigger(doorMap, gridX, gridY, uid,
            [[
                return getUID()
            ]],
            [[
                local questUID = ...
                return function(uid, gridX, gridY)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_enter_cave'})
                    return false
                end
            ]])
        end)
    end,

    quest_enter_cave = function(uid, args)
        if not enterCaves(uid) then
            setQuestState{uid = uid, state = 'quest_kill_boss'}
            return
        end
        setQuestState{uid = uid, state = 'quest_in_cave'}
    end,

    quest_in_cave = function(uid, args)
        setQuestDesp{uid=uid, '在蜈蚣洞窟里往深处走，水源地有污染水源的沃毒蜈蚣。'}
    end,

    -- walked back out of 1_020 without finishing
    quest_left_cave = function(uid, args)
        setQuestState{uid = uid, state = 'quest_kill_boss'}
    end,

    -- [526], the book is already on you and nobody handed it over
    quest_cave_done = function(uid, args)
        closeCaves(uid)
        setQuestDesp{uid=uid, '沃毒蜈蚣死了，回村子跟那个人说一声，再回道馆找大悲善僧。'}

        -- @mugong_massheal_illtown_complete
        setupNPCQuestBehavior(villagerMap, villagerNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '洞窟的事',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我们村庄的人们永远都不会忘记<t color="red">你的善行</t>。。。</par>
                            <par>现在去找每年在这个地方贴护身符并上香的奇怪老头。</par>
                            <par>祝你走运。。。一路小心。。。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '回来复命',

                -- @mugong_massheal_complete0
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>(不对，我为什么在这个地方??? )</par>
                            <par></par>
                            <par><event id="npc_look_around">看看周围...</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete1
                npc_look_around = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊，是你哟。回来了？</par>
                            <par></par>
                            <par><event id="npc_tell_story">那个....在大飞圣僧所讲的地方经历了非常怪异的事情。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete2
                npc_tell_story = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道了。那个村子是<t color="red">百年之前由于传染病而消失了的村子</t>。自从作为那个村子乳汁的溪水被污染后，人们都生病而死。</par>
                            <par></par>
                            <par><event id="npc_they_were_alive">真是无法相信的事情。我分别和那个地方的人们谈话了，他们都是活人。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete3
                npc_they_were_alive = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我以前没有讲过吗？世上的事情中无法说明道理的更多。你遇见的事情也是其中的一种。有可能由于对蜈蚣的憎恨和拯救村子的坚定意志使得<t color="red">那些人的灵魂</t>继续留在那个地方。</par>
                            <par>你看到的东西是他们的灵魂。。你没有感觉到他们不像活着的人吗？</par>
                            <par></par>
                            <par><event id="npc_strong_will">虽然没有感觉到他们生的很好看...感觉到他们有很强的意志，无论如何不能认为是亡灵。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete4
                npc_strong_will = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>有才干哟.你看到的那些东西都是因为你和他们有缘分。你终究做成了我没有做成的事情，哈哈。。</par>
                            <par></par>
                            <par><event id="npc_what_do_you_mean">什么话儿？</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete5
                npc_what_do_you_mean = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>听说这个村子开始流行传染病消息的时候，我还不能给他们任何帮助。我认为世界上没有任何事情比拯救一个村子更有价值的事情了。</par>
                            <par>哈哈，我又在讲废话了。</par>
                            <par></par>
                            <par><event id="npc_yes_it_happened">是的，曾经有过这个事情...</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete6
                npc_yes_it_happened = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯。。那样了，现在可以还给我以前委托你事情的时候给你的<t color="red">威魂深怨护身符</t>吗？</par>
                            <par></par>
                            <par><event id="npc_return_charm">好的，在这儿。</event></par>
                            <par><event id="npc_charm_gone">这个，好像落在哪儿了。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_massheal_complete7_1, and he gives the charm straight back
                npc_return_charm = function(uid, value)
                    if not server.player.hasItem(uid, '威魂深怨护身符', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>噢，听说年轻朋友想笼络老人。。。你没有威魂深怨护身符吗？</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>谢谢！虽然现在不需要，为了以后要好好地保管。</par>
                            <par>好吧，拿着吧。接受这么困难的委托，将贵重的威魂深怨护身符再重新送给你。</par>
                            <par>但是你身上的<t color="red">书籍</t>是什么？</par>
                            <par></par>
                            <par><event id="npc_the_book">不对, 这是群体治愈术的秘诀？这个东西怎么在这儿...</event></par>
                        </layout>
                    ]=])

                    -- take then give, so the charm ends up back with you either way
                    server.player.removeItem(uid, '威魂深怨护身符', 1)
                    server.player.addItem(uid, '神圣铂金戒指', 1)
                    server.player.deliverGold(uid, 10000)
                    server.player.deliverGold(uid, 33000)
                end,

                -- @mugong_massheal_complete7_2, no charm back and 10000 less for it
                npc_charm_gone = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>也没有办法。你也不是故意弄丢的，我再买一个。。。</par>
                            <par>接着，这是接受困难委托的<t color="red">谢礼??</t>。</par>
                            <par>但是你身上的<t color="red">书籍</t>是什么？</par>
                            <par></par>
                            <par><event id="npc_the_book">不对, 这是群体治愈术的秘诀？这个东西怎么在这儿...</event></par>
                        </layout>
                    ]=])

                    server.player.addItem(uid, '神圣铂金戒指', 1)
                    server.player.deliverGold(uid, 33000)
                end,

                -- @mugong_massheal_complete8, SET [730]
                npc_the_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，这个世界上还有不少莫名其妙的事，用这个将解了那些人的怨恨。。。</par>
                            <par>你真的做了好事。将成为其他道士们<t color="red">的很好谈资</t>。。一路顺风。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- @MapQuest_massheal_cave's [730] / [526] / not-[524] / not-[523] branches. the quest's own
-- quest_kill_boss installs an EPUID trigger on the same grids to let its player in, and EPUID
-- wins, so this only ever answers somebody who has no business there
for _, grid in ipairs(doorGrids) do
    for dx = 0, grid[3] - 1 do
        for dy = 0, grid[4] - 1 do
            setupMapDefaultGridTrigger(doorMap, grid[1] + dx, grid[2] + dy,
            [[
                return getUID()
            ]],
            [[
                local questUID = ...
                return function(uid, x, y)
                    local state = server.quest.getState(questUID, {uid = uid})
                    if (state == SYS_DONE) or (state == 'quest_cave_done') then
                        server.player.postString(uid, '(这里是以前击退蜈蚣的洞窟哦...现在洞口被堵上了。)')
                    else
                        server.player.postString(uid, '(好像是什么洞窟入口？现在洞口被堵上了。)')
                    end
                    return false
                end
            ]])
        end
    end
end

-- @mugong_massheal_illtown answers out of the flags whether or not you are on the quest: it has
-- nothing to say to somebody who never took the charm, and it bows to you once it is over
uidRemoteCall(getNPCharUID(villagerMap, villagerNPC), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '搭话',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [730] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>轻轻吹拂的微风中有道人们的<t color="red">正义之心</t>。</par>
                        <par>希望一路顺风。。</par>
                        <par>（向你磕头）</par>
                        <par></par>
                        <par><event id="%s" close="1">请好好地休息！</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- the check [523] 0 branch, before 大悲善僧 has sent you
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>对庸俗的人没有任何话儿好讲哟。</par>
                    <par>你好象有其它的路。</par>
                    <par>快点走你要走的路吧！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])

-- @mugong_massheal, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName, charmPrice,
[[
    local questUID, questName, minQuestLevel, magicName, charmPrice = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼群体治愈术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [730] 1. @mugong_massheal_complete has its own line for afterwards,
            -- 现在那个地方的魂魄都可以安静地睡觉了, and this is @mugong_massheal's
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书吗？那么你为什么还索要？</par>
                        <par>现在那个地方的魂魄都可以安静地睡觉了。。。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_massheal_next1, checkmagic 群体治愈术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你已经练成了群体治愈术，我再没有什么魔法可以教你了，以后再来找我吧。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_massheal_next2, checklevel 31. the two branches describe the magic
            -- differently, one by what it takes and one by the nine people it reaches
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>群体治愈术是最多可以同时治疗9人的<t color="red">高级恢复术</t>。同时治疗几个人气的消耗非常大，因此没有经过相当水平的训练，修炼该武功是非常困难的。</par>
                        <par>嗯。。想学习的想法值得表扬，但修炼的程度好像还不够。修炼一下再来吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>群体治愈术是同时可以治疗很多人的<t color="red">水平很高的恢复术</t>。除了同时可以治疗很多人以外，与恢复术没有很大的不同，因此有人认为群体治愈术不是很了不起的技术。</par>
                    <par>但是每个人体内的气流都不同，可以同时掌握了解几个人气流的事情<t color="red">需要非同一般的精神力</t>。同时治疗几个人气的消耗非常大，因此该武功是没有经过相当水平的训练完全无法修炼的武功。</par>
                    <par></par>
                    <par><event id="npc_ask_teach">请传授我群体治愈术吧！</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_massheal_next3
        npc_ask_teach = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想起你第一次请求我传授的时候了。</par>
                    <par>那时候我真没有想到你会成为这么优秀的道士。我认为你和一般的训练生一样停留在某个阶段，满足于自己的力量并中断了训练。</par>
                    <par>但是你忍受了很困难的训练过程，超出了我的期望。</par>
                    <par>我现在好像没有什么可以传授给你。</par>
                    <par></par>
                    <par><event id="npc_still_need">毫无道理的话。我现在依然需要大飞圣僧的指教。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_massheal_next4
        npc_still_need = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不是这样的。</par>
                    <par>通过学习可以掌握的知识你已经掌握很充分。</par>
                    <par>你认为不足的部分是你以后一边修炼一边要补充的部分。</par>
                    <par>不满足于现状，而且以后也进行专心修炼，终究有一天可以填补上这个部分的。</par>
                    <par>但是不要忘记<t color="red">真正的武功修炼是从现在开始</t>的名言。</par>
                    <par>嘿嘿，老人的废话很多哦。</par>
                    <par>但是以后修炼武功的过程中，如果有难点，请随时来找我。老人我将尽全力帮助你。</par>
                    <par>这么看来。。</par>
                    <par>有一个很重要的<t color="red">委托</t>。</par>
                    <par></par>
                    <par><event id="npc_what_favor">什么事情?</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_massheal_next5
        npc_what_favor = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>我每年这个时候都要<t color="red">去某个村庄祭祖</t>，但是今年有其它的事情不能直接参加祭祖。由于是很重要的祭祖，不能随便委托别人正在苦闷中。如果是你，我信得过好像可以委托你。</par>
                    <par>不是很困难的事情。将我 给的<t color="red">威魂深怨护身符</t>贴到 祭坛 上，然后背诵祭文，仪式就结束了。可以吗？</par>
                    <par></par>
                    <par><event id="npc_accept">好的，我将参加祭祖。</event></par>
                    <par><event id="npc_not_yet">我还不具备办理这种仪式的能力。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_massheal_next6_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>啧啧。。过分谦虚了哟。现在你应该充满自信心的时候还没有到吗？知道了吗？很遗憾，只好找其他的人了。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_massheal_next6_1, SET [523] and the charm. its checkbaggage has no equivalent
        -- in mir2x, addInventoryItem takes whatever it is handed, so 背囊里没有位置了 has no
        -- trigger here
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>哦哦...</par>
                    <par>可以吗？</par>
                    <par>那个村庄位于<t color="red">盟重县东北方向绝命谷入口的附近</t>。</par>
                    <par>这是<t color="red">威魂深怨护身符</t>，将它贴在祭坛上后，请上香。</par>
                    <par>那么就拜托了...</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.player.addItem(uid, '威魂深怨护身符', 1)
            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
