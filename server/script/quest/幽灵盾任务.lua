-- converted from Envir/QuestDiary/MU_taoist/hangma.txt
-- with its monster hook, MonQuest/hangma.txt
--
-- another fetch, and a long talk to get there: 清明子 wants to hear why you want 幽灵盾 before
-- he warns you it can cripple the caster, and only then asks for a 灵珠 off the zombies in the
-- 飞天废矿. he brews it into 无名药, and like asword's 战酒 you have to drink it before he
-- hands the 秘籍 over
--
-- flags: [722] done, [509] sent for the pearl, [510] pearl in hand, [511] potion poured
--
-- two things the legacy data gets inconsistent about, both kept as written:
--
--   the dialogue has him introduce himself as 大飞圣僧 while the monster hook tells you to take
--   the pearl to 清明子, and 清明子 is who actually hosts the quest per merchant.txt
--
--   the eight MapQuest.txt lines that hook 僵尸1 dying in the 飞天废矿 up to this drop are all
--   commented out, so the pearl never drops in legacy and the quest can not be finished. wired
--   here to what those lines say, the same way 苍蝇拍任务 needed

_G.minQuestLevel = 21

_G.magicName = '幽灵盾'
_G.mijiName  = '幽灵盾（秘籍）'

_G.pearlName  = '灵珠'
_G.potionName = '无名药'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

-- random 10, one kill in ten
_G.pearlChance = 10

-- the 飞天废矿 maps from the commented-out MapQuest.txt block
_G.pearlMaps =
{
    '矿石储藏所_D405',
    '废矿南部洞穴_D406',
    '矿山1层_D421',
    '矿山2层_D422',
    '北部矿石储藏所_D435',
    '北部废矿南面洞穴_D436',
    '南部矿山1层_D451',
    '南部矿山2层_D452',
}

setQuestFSMTable(
{
    -- set [509], off to the 飞天废矿
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '清明子要一颗灵珠，去飞天废矿打魔法僵尸弄一颗回来。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '幽灵盾的灵珠',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>在做什么。。不到飞天废矿找<t color="red">‘灵珠’</t>，认为现在是可以磨磨噌噌的时候嘛？现在很多人正在死去。千万快些 ！！</par>
                            <par></par>
                            <par><event id="npc_explain">修炼幽灵盾要做什么？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_hangma_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>若想学幽灵盾，从魔法僵尸那儿找回<t color="red">灵珠</t>即可。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- [510], the pearl is in your pack
    quest_got_pearl = function(uid, args)
        setQuestDesp{uid=uid, '拿到灵珠了，回本馆交给清明子。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交灵珠',
                [SYS_ENTER] = function(uid, value)
                    -- the ELSESAY of @mugong_hangma_getring, you dropped it somewhere
                    if not server.player.hasItem(uid, '灵珠', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你丢失了灵珠哟。。这该怎么办。。。。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>幸运的是已经找到<t color="red">灵珠</t>了哟。好的，现在该是我遵守约定的时候了。请等一下。。</par>
                            <par></par>
                            <par><event id="npc_brew">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_hangma_getring_next, take 灵珠 / give 无名药, SET [511]
                npc_brew = function(uid, value)
                    if not server.player.hasItem(uid, '灵珠', 1) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好的，请喝<t color="red">药水</t>。这个药是用你拿来的灵珠和其它灵验的药材一起加工制成的珍贵药。这个药可以大力提高内力，吃了这个药，在修炼武功的时候不会发生走火入魔的事情。</par>
                            <par></par>
                            <par><event id="%s" close="1">不，如何承受得了这种辛苦?</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '灵珠', 1)
                    server.player.addItem(uid, '无名药', 1)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_got_potion'})
                end,
            }
        ]])
    end,

    -- [511]. @mugong_hangma_getring_next1 turns you away while the potion is still in your pack
    quest_got_potion = function(uid, args)
        setQuestDesp{uid=uid, '喝掉清明子给的无名药，然后回去找他。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领幽灵盾秘籍',
                [SYS_ENTER] = function(uid, value)
                    -- checkitem 无名药 1
                    if server.player.hasItem(uid, '无名药', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你不是已经吃这个药嘛，快点把这个药吃了。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>为天下万民就连自己的生命都可以像棵草一样抛弃的真正英雄，如果这点都做不到，还可以堂堂正正地生活在世上吗？</par>
                            <par>希望你保持慈善的本性，成为为天下民众费心的<t color="red">真正道士</t>。</par>
                            <par></par>
                            <par><event id="npc_take_book">谢谢.</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_hangma_getring_next2
                npc_take_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么就到此为止了，请上路吧！你现在要做的事情还有很多，千万要小心！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.deliverGold(uid, 22000)
                    server.player.addItem(uid, '幽灵盾（秘籍）', 1)
                    server.player.addItem(uid, '松笛', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- MonQuest/hangma.txt, one kill in ten while [509] is set
local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    {
        monster  = '僵尸1',
        map      = pearlMaps,
        state    = SYS_ENTER,
        chance   = pearlChance,
        once     = true,
        give     = pearlName,
        setState = 'quest_got_pearl',
        say      = "（你现在去找清明子，把灵珠带给他，就可以修炼'幽灵盾'……）",
    },
}

-- @mugong_hangma, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- the ELSESAY he gives a non-taoist, twice over in the legacy branches
    local function postWrongJob(uid)
        uidPostXML(uid, questPath,
        [=[
            <layout>
                <par>对不起，施主不是道士，不能修炼该武功，请回去吧！</par>
                <par></par>
                <par><event id="%s" close="1">结束</event></par>
            </layout>
        ]=], SYS_EXIT)
    end

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼幽灵盾',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [722] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书吗？那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>有什么事情找我吗？</par>
                    <par></par>
                    <par><event id="npc_ask_teach">想得到前辈大飞圣僧的指教而来。</event></par>
                    <par><event id="npc_just_greet">随便看看。</event></par>
                </layout>
            ]=])
        end,

        -- the ELSESAY of @mugong_hangma, what he says if you are not here for the武功
        npc_just_greet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>很高兴，我就是大飞圣僧。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @check_rootin, level first and then job, and the low-level branch has its own job check
        npc_ask_teach = function(uid, value)
            if not server.player.hasJob(uid, '道士') then
                postWrongJob(uid)
                return
            end

            -- @mugong_hangma_lowlevel
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嗯，虽然不可以，那时还不具备修炼该武功的能力。后会有期！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_hangma_highlevel_next, checkmagic 幽灵盾
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你已经正在修炼该武功了哟。好象去找寻其它新的武功更好些。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嘿嘿，虽然不知道什么事情，如果我可以帮忙就好了。</par>
                    <par></par>
                    <par><event id="npc_lore1">下一步</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_hangma_highlevel_next2
        npc_lore1 = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯，是这样的。当然按照道友所讲的。最近有魔力的怪兽频繁地出没于各个地方，对此魔力没有任何抵抗力的很多人正在遭受磨难。</par>
                    <par>因此按照道友所讲是为了帮助这些人才想学习<t color="red">幽灵盾</t>的。</par>
                    <par>嗯，为了他人而献身是我们道士所追求的基本精神，道友正在保持<t color="red">真正道士的精神</t>。我为你的善良品格而感动。</par>
                    <par></par>
                    <par><event id="npc_warning">教我武功吗？</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_hangma_highlevel_next3, the warning you can back out of
        npc_warning = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>使用这种武功时，会从施功者体内消耗巨大的功力，但长时间修炼功力强者没关系。没有此功力者将功力同化，身体无法动弹。严重时会损伤气脉成为废人。</par>
                    <par>为了防止这种危险，虽然已经使用护身符和辅助工具，但是如果发功者的内力不优秀完全起不到任何作用。这样还想学习该武功吗？</par>
                    <par></par>
                    <par><event id="npc_resolved">已经做好了献出生命的准备。</event></par>
                    <par><event id="npc_reconsider">重新考虑一下！</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_hangma_highlevel_next3_except
        npc_reconsider = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>好的</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_hangma_highlevel_next4, and he asks for the favour before teaching
        npc_resolved = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>已经下了这么大的决心也是没有办法的事情。好的！我将传授幽灵盾给你。但是这之前帮我做一件事情可以吗？</par>
                    <par></par>
                    <par><event id="npc_accept_favor">好的。</event></par>
                    <par><event id="npc_refuse_favor">不行。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_hangma_highlevel_next5_except
        npc_refuse_favor = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯。。如果那样，我也不能将武功传授给你。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_hangma_highlevel_next5, set [509]
        npc_accept_favor = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不是其它的事情，听说过生活在飞天费矿的魔法僵尸吗？如果抓到<t color="red">魔法 僵尸</t>偶而会有叫做<t color="red">'灵珠'</t>的奇特珠子出来，请将这个东西拿给我。请不要问这个东西用在哪儿和为什么需要。</par>
                    <par>只要将这个珠子拿来，将传授武功给你。好了，请快去快回！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
