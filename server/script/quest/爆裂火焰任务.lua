-- converted from Envir/QuestDiary/MU_wizard/pokyel.txt
-- with its monster hook, MonQuest/pokyel.txt, registered in Envir/MapQuest.txt against
-- 七点白蛇 dying in 毒蛇山谷_2
--
-- 化天先生 is the second wizard teacher in 银杏山谷 and this is the top of the wizard line at
-- level 32. no training ground: he wants a 七点白蛇胆 out of 毒蛇山谷, brews it into 胆汁, and
-- you have to drink that before he parts with the 秘籍 — the same shape as asword's 战酒 and
-- hangma's 无名药
--
-- he tells you not to use magic on the snake or the gall is ruined. legacy never checks that,
-- the MonDie hook fires however it died, so it stays flavour here too
--
-- the hook's own line says to take the gall to 霹雳尊者, who is the other wizard teacher.
-- 化天先生 is who hosts this one per merchant.txt, and the line is kept as written
--
-- @mugong_fireware_complete_next1 has an ELSESAY for [528] not being set, 我好像还没有给你
-- 胆汁。。。奇怪的的事情。。。, which is legacy's own can-not-happen branch — its text says so.
-- pouring the 胆汁 and moving to quest_got_potion are the same step here, so it has no path
--
-- flags: [764] done, [526] sent for the gall, [527] gall in hand, [528] 胆汁 poured,
-- [529] ready to be paid

_G.minQuestLevel = 32

_G.magicName = '爆裂火焰'
_G.mijiName  = '爆裂火焰（秘籍）'

_G.gallName   = '七点白蛇胆'
_G.potionName = '胆汁'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '化天先生_1'

-- the ramp in MonQuest/pokyel.txt: the counter opens at 3 and the gall drops once it passes 4
_G.gallKills = 4
_G.gallMap   = '毒蛇山谷_2'

setQuestFSMTable(
{
    -- set [526], off to 毒蛇山谷
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '化天先生要一颗七点白蛇胆，去毒蛇山谷打七点白蛇弄一颗回来。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '爆裂火焰的蛇胆',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快点到毒蛇山村寻找七点白蛇的胆汁来。</par>
                            <par></par>
                            <par><event id="npc_explain">修炼爆裂火焰要做什么？</event></par>
                            <par><event id="npc_where">毒蛇山村在哪里？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_fireware_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想学习爆裂火焰，请到毒蛇山村找到<t color="red">七点白蛇胆汁</t>即可。</par>
                            <par>我将利用你找来的蛇胆为材料制成<t color="red">蛇胆汁</t>，喝了这个药后就可以学习该武功了。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_fireware_next7, still worth asking twice
                npc_where = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>过了银杏山谷、比奇县，毒蛇山村就到了。</par>
                            <par>坐标？ 已经达到像你一样的等级了，还不知道吗？</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- [527], the gall is in your pack
    quest_got_gall = function(uid, args)
        setQuestDesp{uid=uid, '拿到七点白蛇胆了，回银杏山谷交给化天先生。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交七点白蛇胆',
                [SYS_ENTER] = function(uid, value)
                    -- the ELSESAY of @mugong_fireware_complete
                    if not server.player.hasItem(uid, '七点白蛇胆', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>时间很重要。不好慢腾腾的，快点找来<t color="red">七点白蛇的胆汁</t>吧。。。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯,很幸运地找来了。好的，现在该我制药了。请等一下！</par>
                            <par></par>
                            <par><event id="npc_brew">下一步</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_fireware_complete_next, SET [528]
                npc_brew = function(uid, value)
                    if not server.player.hasItem(uid, '七点白蛇胆', 1) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>喂，这里有药水。这个药水是用你拿来的<t color="red">胆汁制成的</t>。你吃药的过程中，我将准备武功秘籍。</par>
                            <par></par>
                            <par><event id="%s" close="1">下一步</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '七点白蛇胆', 1)
                    server.player.addItem(uid, '胆汁', 1)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_got_potion'})
                end,
            }
        ]])
    end,

    -- [528] and [529]. @mugong_fireware_complete_next2 turns you away while the 胆汁 is still
    -- in your pack, so the last step is to go and drink it
    quest_got_potion = function(uid, args)
        setQuestDesp{uid=uid, '喝掉化天先生给的胆汁，然后回去找他。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '领爆裂火焰秘籍',
                [SYS_ENTER] = function(uid, value)
                    -- checkitem 胆汁 1
                    if server.player.hasItem(uid, '胆汁', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你现在还没有吃<t color="red">药</t>，如果这样我也不能把书给你。</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    -- @mugong_fireware_complete_next3
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>喝完这个药后，掌握了可以解毒的武功书就不会出现走火入魔的事情了。</par>
                            <par>希望你可以将武功用在有用的事情上。</par>
                            <par></par>
                            <par><event id="npc_take_book" close="1">谢谢！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_book = function(uid, value)
                    server.player.addItem(uid, '爆裂火焰（秘籍）', 1)
                    server.player.deliverGold(uid, 99000)
                    server.player.addItem(uid, '流星天玉', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- MonQuest/pokyel.txt, 七点白蛇 in 毒蛇山谷 while [526] is set
local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    {
        monster  = '七点白蛇',
        map      = gallMap,
        state    = SYS_ENTER,
        kills    = gallKills,
        once     = true,
        give     = gallName,
        setState = 'quest_got_gall',
        say      = "（把'七点白蛇胆汁'带给霹雳尊者，这样就可以修炼魔法了....）",
    },
}

-- @mugong_fireware, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    -- the ELSESAY he gives a non-wizard, the same line from both the level branches
    local function postWrongJob(uid)
        uidPostXML(uid, questPath,
        [=[
            <layout>
                <par>对不起，你还不是魔法师吗？你不能学习该武功，请回吧！</par>
                <par></par>
                <par><event id="%s" close="1">结束</event></par>
            </layout>
        ]=], SYS_EXIT)
    end

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼爆裂火焰',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [764] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书了吗？那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你有什么事吗？说说看。。</par>
                    <par>嗯，想学称为“爆裂火焰”的武功？</par>
                    <par></par>
                    <par><event id="npc_ask_teach">下一步</event></par>
                    <par><event id="npc_just_greet">没什么事。</event></par>
                </layout>
            ]=])
        end,

        -- the ELSESAY of @mugong_fireware, what he says if you are not here for the武功
        npc_just_greet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>如果需要帮忙，请随时来找我！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_fireware_next, level first and then job, and the low-level branch checks the
        -- job too so a non-wizard hears about the job rather than the level
        npc_ask_teach = function(uid, value)
            if not server.player.hasJob(uid, '法师') then
                postWrongJob(uid)
                return
            end

            -- @mugong_fireware_next_lowlevel
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嗯。。你现在学习该武功还是有些早。提高武功等级后再来吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- @mugong_fireware_next1, checkmagic 爆裂火焰
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经修炼了该武功吗..请找寻其它的武功吧！</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>作为一名魔法师，你<t color="red">上一个台阶</t>的时机终于来了。</par>
                    <par></par>
                    <par><event id="npc_offer_lore">下一步</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next2
        npc_offer_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想听对该武功的说明吗？</par>
                    <par></par>
                    <par><event id="npc_lore">是的，想听。</event></par>
                    <par><event id="npc_no_lore">不需要。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next3_1
        npc_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>魔法师在1对1的斗争中是最强的，但是遇到多数敌人的包围马上就变成了守势。即使不遭到包围，体力和气力很快地消耗，因此不能进行长期战。为了弥补这种魔法师的缺点而产生的武功正是<t color="red">'爆裂火焰'</t>。</par>
                    <par>“爆裂火焰“是在<t color="red">一定范围之内可以产生火焰大爆炸</t>的技术。这周围所在的敌人将受到很大的破坏。虽然能源的消耗大，如果熟练的话反而可以节省能源。</par>
                    <par>对于分散开的敌人没有什么作用。虽然有在使用该技术之前要将<t color="red">敌人引诱到一个地方的缺点</t>，<t color="red">对移动快捷的敌人进行攻击</t>还是很有效。效果显著，是任何武功都比不上的。</par>
                    <par></par>
                    <par><event id="npc_ask_learn">请教我该武功吧！</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next3_2
        npc_no_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那样？你对我的希望是什么？</par>
                    <par></par>
                    <par><event id="npc_ask_learn">请教我该武功吧！</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next4, the warning you can back out of
        npc_ask_learn = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>嗯，虽然不可以，也不得不这样了！</par>
                    <par>我看你练习该武功<t color="red">内力</t>还是有些不足，练习武功之前，内力不能抑制火气的话，将走火入魔。失去武功固然不好，有时候有可能丧失生命。那还要练习吗？</par>
                    <par></par>
                    <par><event id="npc_resolved">即使有失去生命的遗憾，也要练习。</event></par>
                    <par><event id="npc_not_yet">现在好象有些勉强。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next5_2
        npc_not_yet = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>没有办法。如果认为很勉强，不做也是其中的一个方法。。。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_fireware_next5_1
        npc_resolved = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>已经下了这么大的决心，我教你一种防御方法。</par>
                    <par></par>
                    <par><event id="npc_send_for_gall">下一步</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next6
        npc_send_for_gall = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你沿着这条路去<t color="red">毒蛇山村</t>，找到七点白蛇，并拿到它的胆汁。用<t color="red">七点白蛇的胆汁</t>制成药，服下此药，内力可以大增，而且可以抑制火气逆行。</par>
                    <par>而且有重要的注意事项，<t color="red">在抓七点白蛇时千万不可以使用魔法。</t>如果使用了魔法，蛇胆被破坏将破坏药效，一定要直接进攻捕到毒蛇。</par>
                    <par>你如果找来七点白蛇胆汁，我将给你制作增强内力的<t color="red">仙丹</t>还传授给你武功。</par>
                    <par>还有疑问吗？</par>
                    <par></par>
                    <par><event id="npc_where">毒蛇山村在哪里？</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next7
        npc_where = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>过了银杏山谷、比奇县，毒蛇山村就到了。</par>
                    <par>坐标？ 已经达到像你一样的等级了，还不知道吗？</par>
                    <par></par>
                    <par><event id="npc_why">为什么需要七点白蛇的胆汁？</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next8
        npc_why = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>爆裂火焰的火力非常强大。在修炼不足的情况下修炼该武功，体内的火魔将逆行，从而伤害内脏器官。我年轻的时候也是抑制不住冲动，仓促修炼该武功，从而受到内伤，到现在为止还受到伤痛的折磨。</par>
                    <par>用七点白蛇的胆汁制成药，吃了以后可以增强内力，抑制体内的火气逆行。</par>
                    <par></par>
                    <par><event id="npc_accept">知道了。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireware_next9, set [526]
        npc_accept = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么，快点去找到<t color="red">蛇胆汁</t>吧。这期间我准备其他的药材。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
