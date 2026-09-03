-- converted from Envir/QuestDiary/MU_wizard/lightLine.txt
-- with its two hooks, MonQuest/lightLine1.txt and MonQuest/lightLine2.txt, registered in
-- Envir/MapQuest.txt against 洞蛆 in 天然洞穴1层 and 山洞蝙蝠 in the 沃玛神殿 entrances
--
-- the longest chain of the wizard quests and the only one that can cost you something. a
-- 闪电石 burns to hold, so before you can pick one up 布店晓芙 has to coat your 魔法长袍 in
-- 树脂 — and she only manages it half the time. when she fails the robe is gone and so is the
-- 树脂, and you go back to the caves for another
--
-- steps: 洞蛆 in 天然洞穴1层 drop 树脂 one time in ten, 布店晓芙 coats the robe, 山洞蝙蝠 at
-- the 沃玛神殿 entrances drop 闪电石 one time in twenty, and 霹雳尊者 trades that for the 秘籍
--
-- flags: [757] done, [522] sent for the resin, [523] resin in hand, [524] robe coated,
-- [525] stone in hand
--
-- the hook lines credit 华川先生 for the advice and call 布店晓芙 the 棉布商 or 福氏, none of
-- which is what either NPC is called anywhere else. kept as written

_G.minQuestLevel = 21

_G.magicName = '疾光电影'
_G.mijiName  = '疾光电影（秘籍）'

_G.resinName = '树脂'
_G.stoneName = '闪电石'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

_G.tailorMap = '银杏山谷_02'
_G.tailorNPC = '布店晓芙_1'

-- random 10 in lightLine1, random 20 in lightLine2
_G.resinChance = 10
_G.stoneChance = 20

_G.resinMap  = '天然洞穴1层_D011'
_G.stoneMaps =
{
    '沃玛神殿入口_D021',
    '沃玛神殿1层_D022',
    '沃玛神殿1层_D032',
    '沃玛神殿1层_D042',
    '沃玛神殿1层_D052',
}

-- she only works on a 魔法长袍, and only the one you are wearing
local function robeName(uid)
    return server.player.getGender(uid) and '魔法长袍（男）' or '魔法长袍（女）'
end

local function coatedRobeName(uid)
    return server.player.getGender(uid) and '树脂魔法长袍（男）' or '树脂魔法长袍（女）'
end

-- the [522] branch of @mugong_lightline, all he says until the stone turns up
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
            [SYS_LABEL] = '疾光电影的闪电石',
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>一定要用树脂给衣服穿上薄膜后才可以得到电雷草，不要忘了这点。</par>
                        <par></par>
                        <par><event id="npc_explain">这件事要怎么做？</event></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            -- @mugong_lightline_explain
            npc_explain = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想修炼疾光电影，首先到天然洞穴中找到<t color="red">树脂</t>后，再到银杏树村请棉布商给你的<t color="red">魔法长袍</t>涂上树脂。</par>
                        <par>如果得到了树脂火焰魔衣，就可以在天然洞穴里找到<t color="red">闪电石</t>，然后把闪电石拿给我就可以了。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,
        }
    ]])
end

-- @mugong_lightline_suzi, 布店晓芙's coin flip
local function setupTailor(uid)
    setupNPCQuestBehavior(tailorMap, tailorNPC, uid,
    string.format([[ return getUID(), getQuestName(), %s, %s ]], asInitString(robeName(uid)), asInitString(coatedRobeName(uid))),
    [[
        local questUID, questName, robeName, coatedRobeName = ...
        local questPath = {SYS_EPUID, questName}

        return
        {
            [SYS_LABEL] = '给魔法长袍涂树脂',
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>现在我就给你穿的衣服上涂<t color="red">树脂</t>，但能否成功我也不知道。。。如果成功了，您衣服的耐久好象可以修理了。</par>
                        <par>还有如果衣服上涂了树脂，你衣服的<t color="red">基本功能就消失并成为了一般的耐久</t>。请注意这点。。。但是由于具有了树脂的功能，也就拥有了<t color="red">特殊的功能</t>。</par>
                        <par>我现在就试着涂一下，请等一下！</par>
                        <par></par>
                        <par><event id="npc_coat">下一步</event></par>
                    </layout>
                ]=])
            end,

            -- @mugong_lightline_suzi_man1 / _wman1: the 树脂 in your pack and the robe on your
            -- back, both, and the man branch calls you by name where the woman branch does not
            npc_coat = function(uid, value)
                local worn = server.player.getWLItem(uid, WLG_DRESS)
                local wearingRobe = worn ~= nil and worn.itemID == getItemID(robeName)

                if not (server.player.hasItem(uid, '树脂', 1) and wearingRobe) then
                    if server.player.getGender(uid) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>现在穿的衣服，只有<t color="red">魔法长袍</t>才可以涂上树脂。。我看 %s 先生没有穿魔法长袍或者没有树脂了。。。</par>
                                <par>树脂可以在<t color="red">天然洞穴1层 洞蛆</t>找到。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], server.player.getName(uid), SYS_EXIT)
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>现在穿的衣服，只有<t color="red">魔法长袍</t>才可以涂上树脂。。我看您没有穿魔法长袍或者没有树脂了。。。</par>
                                <par>树脂可以在天然洞穴1层找到。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                    return
                end

                -- both go in whether or not it works
                server.player.removeItem(uid, '树脂', 1)
                server.player.removeWearItem(uid, WLG_DRESS)

                -- random 2
                if math.random(2) ~= 1 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这个怎么办。。。。涂树脂的过程中<t color="red">将衣服破坏了。。。</t>这如何是好…对不起。。。如果重新再找到的话，我再给你做。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    -- SET [523] 0, back to the caves for another 树脂
                    server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
                    return
                end

                -- @mugong_lightline_suzi_man3 / _wman3, SET [524]
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>恭喜你<t color="red">成功了。。。</t>虽然不知道这是用在那里的东西。。。请好好使用。。。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)

                server.player.addItem(uid, coatedRobeName, 1)
                server.quest.setState(questUID, {uid = uid, state = 'quest_find_stone'})
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- SET [522], and where the failed coating drops you back to
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '去天然洞穴1层打洞蛆找树脂。'}
        setupTeacherNag(uid)
    end,

    -- [523], the resin is in your pack
    quest_got_resin = function(uid, args)
        setQuestDesp{uid=uid, '拿到树脂了，穿好魔法长袍去找银杏山谷的布店晓芙涂树脂。'}
        setupTeacherNag(uid)
        setupTailor(uid)
    end,

    -- [524], the robe is coated and the stone will not burn you
    quest_find_stone = function(uid, args)
        setQuestDesp{uid=uid, '树脂魔法长袍做好了，去沃玛神殿入口打山洞蝙蝠找闪电石。'}
        setupTeacherNag(uid)
    end,

    -- [525], the stone is in your pack
    quest_got_stone = function(uid, args)
        setQuestDesp{uid=uid, '拿到闪电石了，回去交给霹雳尊者。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            -- the ELSESAY of @mugong_lightline_test_next1
            local function postLostStone(uid)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>很困难才求得的<t color="red">闪电石</t>，我把它放在其它地方了。下一次带来。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end

            return
            {
                [SYS_LABEL] = '交闪电石',

                -- @mugong_lightline_test
                [SYS_ENTER] = function(uid, value)
                    if not server.player.hasItem(uid, '闪电石', 1) then
                        postLostStone(uid)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在感到体内有内力了，已经修炼成<t color="red">闪电石</t>的样子。</par>
                            <par></par>
                            <par><event id="npc_take_book">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_lightline_test_next1 and next2
                npc_take_book = function(uid, value)
                    if not server.player.hasItem(uid, '闪电石', 1) then
                        postLostStone(uid)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>辛苦了！这里有疾光电影秘籍，请看着练习就可以了。以后要修炼的武功还很多，别骄傲，请继续练习！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '闪电石', 1)
                    server.player.addItem(uid, '疾光电影（秘籍）', 1)
                    server.player.deliverGold(uid, 26000)
                    server.player.addItem(uid, '月光石手镯', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    -- lightLine1, 洞蛆 in 天然洞穴1层 while [522] is set and [523] is not
    {
        monster  = '洞蛆',
        map      = resinMap,
        state    = SYS_ENTER,
        chance   = resinChance,
        once     = true,
        give     = resinName,
        setState = 'quest_got_resin',
        say      = '（这是华川先生所讲的树脂吗？那么现在就要去银杏树村，请棉布商给衣服涂树脂了。）',
    },

    -- and the [523] branch, which just reminds you where to take it
    {
        monster = '洞蛆',
        map     = resinMap,
        state   = 'quest_got_resin',
        say     = '（要到银杏树村的棉布商那儿，请她给衣服上涂树脂，我在这里做什么呢？）',
    },

    -- lightLine2, 山洞蝙蝠 at the 沃玛神殿 entrances once the robe is coated
    {
        monster  = '山洞蝙蝠',
        map      = stoneMaps,
        state    = 'quest_find_stone',
        chance   = stoneChance,
        once     = true,
        give     = stoneName,
        setState = 'quest_got_stone',
        say      = '（这就是华川先生所讲的闪电石吗？如果没有涂树脂将如何提这个东西。。手上火辣辣的。）',
    },

    -- and the [525] branch
    {
        monster = '山洞蝙蝠',
        map     = stoneMaps,
        state   = 'quest_got_stone',
        say     = '(现在该回到化天先生那里了。)',
    },
}

-- @mugong_lightline, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼疾光电影',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [757] 1. the legacy line asks the question inverted, kept as written
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有收到疾光电影秘籍吗? 那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 疾光电影
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>魔法师在修炼新武功的时候是不可以偷懒的。如果偷懒，瞬间之内将受到不可治愈的伤害。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_lightline_next, checklevel 21. below it he still describes the magic,
            -- which is the one place that description appears
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>使用闪电石力量的武功除了你已经掌握的雷电术之外，还有<t color="red">疾光电影</t>。是一种<t color="red">以进攻者为准，闪电石之力以一条直线的形式发射出去的武功</t>。尤其是对<t color="red">狮子类</t>很有效果，好好掌握和利用。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_lightline_next3
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想学习称为疾光电影的武功？</par>
                    <par>要修炼疾光电影的武功需要高水平的<t color="red">闪电石</t>。</par>
                    <par>但是我看你好像还没有这么大的力量。</par>
                    <par></par>
                    <par><event id="npc_ask_how">那么如何才可以拥有你称为“闪电石”的能力？</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightline_next4
        npc_ask_how = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>把沃玛神殿的<t color="red">闪电石</t>带来就可以了。但是听说，闪电石不能那么简单地拿到。。叫什么。。抓到蛆，就可以得到<t color="red">树脂</t>，然后到<t color="red">银杏树村福氏</t>，请他将树脂涂到你的衣服上。这样才可以战胜电雷草的闪电石，从而得到闪电石。</par>
                    <par>是说获得树脂的地方吗？曾经听说<t color="red">天然洞穴1层 洞蛆</t>中有树脂。还有传说讲<t color="red">沃玛神殿入口 山洞蝙蝠</t>拥有闪电石。</par>
                    <par></par>
                    <par><event id="npc_ask_why">学习武功，为什么这么困难？</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightline_next5
        npc_ask_why = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果世上的事情都很简单，还有掌握武功的必要吗？要记住容易获得的东西，也容易失去。</par>
                    <par></par>
                    <par><event id="npc_accept">知道了。那就试一次吧。</event></par>
                    <par><event id="npc_not_yet">我还是喜欢简单的东西。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_lightline_next6_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果你意如此，我也不再劝阻。如果你的想法变了，请再来！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_lightline_next6_1, SET [522]
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想得不错，获得闪电石虽然辛苦，完成了此事成就感也就比较大。那么就快去快回吧！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
