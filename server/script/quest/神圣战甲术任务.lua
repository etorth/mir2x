-- converted from Envir/QuestDiary/MU_taoist/deaji.txt
-- with its two hooks, MonQuest/deaji1.txt and MonQuest/deaji2.txt, registered in
-- Envir/MapQuest.txt against the 比奇废矿 maps and 神圣战甲术空间_D400_001
--
-- 清明子 wants a 起爆石 to keep the nature-qi from turning on you, and it is two hops away:
-- one zombie in fifty anywhere in the 比奇废矿 drops a plain 神圣战甲术 book and the book pulls
-- you into a hidden room, and one 尸王 in ten in that room has the stone
--
-- 尸王 killed again once you have the stone just puts you back out at the mine entrance
--
-- the room is ordinary content rather than a private trial — legacy never refused anyone at the
-- door and the 尸王 in there is not spawned by the quest — so it stays the shared map
--
-- flags: [725] done, [515] sent to the mine, [516] stone in hand
--
-- deaji.txt also carries a @MapQuest_Upac_Recall2 that is the same hook with random 100 and
-- 神圣战甲术空间_D420_001 instead. no MapQuest.txt line ever calls it, so it is dead in the
-- legacy data and D420_001 goes unused
--
-- @mugong_Upac never checks the job, only the level and the magic. kept that way

_G.minQuestLevel = 25

_G.magicName = '神圣战甲术'
_G.mijiName  = '神圣战甲术（秘籍）'
_G.bookName  = '神圣战甲术'
_G.stoneName = '起爆石'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

-- random 50 in deaji1, random 10 in deaji2
_G.bookChance  = 50
_G.stoneChance = 10

-- the hidden room the book drops you into, and the entrance 尸王 sends you back to
_G.secretMap = '神圣战甲术空间_D400_001'
_G.mineEntry = '废矿矿山入口_D401'

-- every 比奇废矿 map that hooks deaji1
_G.mineMaps =
{
    '废矿矿山入口_D401',
    '废矿东部洞穴_D402',
    '地下1层采矿所_D403',
    '地下2层采矿所_D404',
    '矿石储藏所_D405',
    '废矿南部洞穴_D406',
    '地下1层第一间房_D411',
    '天桥2_D412',
    '地下1层第二间房_D413',
    '地下1层第三间房_D414',
    '天桥1_D415',
    '天桥3_D416',
}

-- the MapQuest lines name a different set of zombies per map, this is the union of them
_G.mineZombies =
{
    '僵尸1', '僵尸_1', '僵尸3', '僵尸4', '僵尸5',
    '僵尸10', '僵尸20', '僵尸30', '僵尸40', '僵尸50',
    '僧侣僵尸', '雷电僵尸',
}

-- @mugong_Upac_next6, the same nudge from both the [515] branch and the end of the briefing
local function setupMineNag(uid)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    [[
        return getQuestName()
    ]],
    [[
        local questName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '神圣战甲术的起爆石',
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>请在<t color="red">比奇废矿</t>仔细找一下吧。那你就可以在秘密地点找到<t color="red">起爆石</t>。</par>
                        <par></par>
                        <par><event id="npc_explain">这件事要怎么做？</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_Upac_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>要想修炼神圣战甲术, 首先要从比奇废矿的怪物身上夺回<t color="red">神圣战甲术</t>。</par>
                        <par>必须把你打败的僵尸掉下的神圣战甲术书放入包里，这样你就会被移动到废矿的秘密房间。</par>
                        <par>那个房间是<t color="red">尸王</t>居住的地方, 在这些怪物当中就可以得到<t color="red">起爆石</t>。</par>
                        <par>把起爆石给我带来即可。</par>
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
    -- SET [515], off to the 比奇废矿
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '去比奇废矿打僵尸，捡到神圣战甲术书就会被带进秘密房间，在那里找起爆石。'}
        setupMineNag(uid)
    end,

    -- the book dropped and pulled you into the hidden room
    quest_in_secret_room = function(uid, args)
        setQuestDesp{uid=uid, '在秘密房间里打尸王，拿到起爆石。'}
        setupMineNag(uid)
    end,

    -- [516], the stone is in your pack
    quest_got_stone = function(uid, args)
        setQuestDesp{uid=uid, '拿到起爆石了，回本馆交给清明子。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            -- the two ELSESAYs of @mugong_Upac_test and its next, both about losing the stone
            local function postLostStone(uid, wording)
                uidPostXML(uid, questPath, wording, SYS_EXIT)
            end

            return
            {
                [SYS_LABEL] = '交起爆石',
                [SYS_ENTER] = function(uid, value)
                    if not server.player.hasItem(uid, '起爆石', 1) then
                        postLostStone(uid,
                        [=[
                            <layout>
                                <par>你把起爆石给弄丢了?? 没有起爆石我可无法让你修炼神圣战甲术..</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=])
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好好.. 这个<t color="red">起爆石</t>就是调节大自然和你之间真气的石头。当你拿到这颗石头的同时，你已经拥有了调节大自然和你之间真气的能力..</par>
                            <par>你现在有充分的能力修炼神圣战甲术了...</par>
                            <par></par>
                            <par><event id="npc_take_book">那么请给我神圣战甲术(秘籍)吧</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_Upac_test_next, take 起爆石, then next3 or next4 on gender
                npc_take_book = function(uid, value)
                    if not server.player.hasItem(uid, '起爆石', 1) then
                        postLostStone(uid,
                        [=[
                            <layout>
                                <par>你丢了<t color="red">起爆石</t>?? 那我可没办法让你修炼神圣战甲术..</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=])
                        return
                    end

                    -- gender man. the closing line differs by a red mark on 大自然真气
                    if server.player.getGender(uid) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>想想你已经修炼成功的魔法，你就会知道该怎么使用神圣战技术了，持续使用神圣战甲术，你就自然而然学会利用大自然真气的。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>想想你已经修炼成功的魔法，你就会知道该怎么使用神圣战技术了，持续使用神圣战甲术，你就自然而然学会利用<t color="red">大自然真气</t>的。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end

                    server.player.removeItem(uid, '起爆石', 1)
                    server.player.addItem(uid, '神圣战甲术（秘籍）', 1)
                    server.player.addItem(uid, server.player.getGender(uid) and '神奇灵魂战衣（男）' or '神奇灵魂战衣（女）', 1)
                    server.player.addItem(uid, '八面太极戒指', 1)
                    server.player.deliverGold(uid, 25000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    -- deaji1, one zombie in fifty anywhere in the mine. the book is the key to the room and
    -- the same step that hands it over pulls you in
    {
        monster  = mineZombies,
        map      = mineMaps,
        state    = SYS_ENTER,
        chance   = bookChance,
        give     = bookName,
        setState = 'quest_in_secret_room',
        moveTo   = {secretMap},
    },

    -- deaji2, one 尸王 in ten has the stone
    {
        monster  = '尸王',
        map      = secretMap,
        state    = 'quest_in_secret_room',
        chance   = stoneChance,
        once     = true,
        give     = stoneName,
        setState = 'quest_got_stone',
        say      = '(原来这就是起爆石啊... 得赶快带给清明子..)',
    },

    -- and the [516] branch of deaji2, which just shows you the door
    {
        monster = '尸王',
        map     = secretMap,
        state   = 'quest_got_stone',
        moveTo  = {mineEntry},
    },
}

-- @mugong_Upac, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼神圣战甲术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [725] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经有了神圣战甲术(秘籍)吗? 为什么还跟我要呢?</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 神圣战甲术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经修炼了神圣战甲术吗?</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_Upac_next1, checklevel 25
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有能力修炼神圣战甲术..到了<t color="red">等级 %d</t>再来找我吧。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            -- @mugong_Upac_next3
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>使用神圣战甲术可以瞬间吸收大自然的真气，<t color="red">提高物理防御力</t>。</par>
                    <par>你想修炼神圣战甲术吗?要想修炼神圣战甲术就要学会吸收大自然真气的方法。世上万物各有各的真气，神圣战甲术就是吸收这种真气，<t color="red">一定时间内保护自己</t>。</par>
                    <par></par>
                    <par><event id="npc_ask_teach">教我神圣战甲术吧</event></par>
                    <par><event id="npc_not_yet">我想我还未做好准备</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_Upac_next4_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯....为了修炼新的技术而变换自己或许是可怕的事情。做好心理准备之后再来吧。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_Upac_next4_1, the warning that there is a catch
        npc_ask_teach = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>前面我也说了，要想修炼神圣战甲术就要学会吸收<t color="red">大自然真气</t>的方法，你还没有这个能力，小心走火入魔啊。</par>
                    <par></par>
                    <par><event id="npc_ask_how">不走火入魔还可修炼神圣战甲术该怎么做呢?</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_Upac_next5, SET [515]
        npc_ask_how = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>吸收大自然真气时，不走火入魔的方法之一是找到可以协调大自然和你之间真气的<t color="red">协调物</t>。此协调物要有最<t color="red">洁净的大自然真气</t>..</par>
                    <par>这个协调物隐藏在<t color="red">起爆石</t>里面.. 起爆石可在带着<t color="red">比奇废矿</t>所得到的某种特殊物品就可进入的秘密地点获得。</par>
                    <par></par>
                    <par><event id="npc_accept">下一个</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_Upac_next6
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>请在<t color="red">比奇废矿</t>仔细找一下吧。那你就可以在秘密地点找到<t color="red">起爆石</t>。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
