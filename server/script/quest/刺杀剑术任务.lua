-- converted from Envir/QuestDiary/MU_warrior/asword.txt
-- with its monster hook, MonQuest/asword.txt, registered in Envir/MapQuest.txt against
-- 沃玛战士 dying on the six 沃玛神殿 maps
--
-- no training ground for this one. 龙血先生 talks you through what 刺杀剑术 is and why it is
-- kept quiet, tests your nerve with a question you can answer wrong, then sends you into
-- 沃玛神殿 for a 沃玛角. he brews it into 战酒, and you have to actually drink it before he
-- will part with the 秘籍
--
-- flags: [702] done, [503] sent for the horn, [504] horn in hand, [505] 战酒 poured

_G.minQuestLevel = 19

_G.magicName = '刺杀剑术'
_G.mijiName  = '刺杀剑术（秘籍）'

_G.hornName = '沃玛角'
_G.wineName = '战酒'

_G.teacherMap = '边境城市_01'
_G.teacherNPC = '龙血先生_1'

-- the ramp in MonQuest/asword.txt: the counter opens at 3 and the horn drops once it passes 10
_G.hornKills = 10

_G.hornMaps =
{
    '沃玛神殿1层_D022',
    '沃玛神殿2层_D023',
    '沃玛神殿1层_D032',
    '沃玛神殿2层_D033',
    '沃玛神殿1层_D042',
    '沃玛神殿2层_D043',
}

-- @mugong_asword_next11 and the ELSESAY of @mugong_asword_complete, the same nag
local function nagBehavior(uid)
    setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
    [[
        return getQuestName()
    ]],
    [[
        local questName = ...
        local questPath = {SYS_EPUID, questName}
        local dialog = require('include.dialog')

        return
        {
            [SYS_LABEL] = '刺杀剑术的沃玛角',
            [SYS_ENTER] = function(uid, value)
                dialog.post(uid, questPath, '做什么呢？不快点找<t color="red">沃玛角</t>。',
                {
                    dialog.link('npc_explain', '修炼刺杀剑术要做什么？'),
                    dialog.link(SYS_EXIT, '结束'),
                })
            end,

            -- @mugong_asword_explain
            npc_explain = function(uid, value)
                dialog.post(uid, questPath,
                {
                    '要想修炼刺杀剑术, 就要到沃玛神殿找来<t color="red">沃玛角</t>。',
                    '我会把你带来的沃玛角磨成粉制作<t color="red">战酒</t>，喝了它就可以修炼刺杀剑术。',
                },
                dialog.link(SYS_EXIT, '结束'))
            end,
        }
    ]])
end

setQuestFSMTable(
{
    -- set [503], off to 沃玛神殿
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '龙血先生要一只沃玛角，去沃玛神殿打沃玛战士弄一只回来。'}
        nagBehavior(uid)
    end,

    -- [504], the horn is in your pack
    quest_got_horn = function(uid, args)
        setQuestDesp{uid=uid, '拿到沃玛角了，回边境城市找龙血先生。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_LABEL] = '交沃玛角',
                [SYS_ENTER] = function(uid, value)
                    -- he checks again, the horn could be gone
                    if not server.player.hasItem(uid, '沃玛角', 1) then
                        dialog.post(uid, questPath, '做什么呢？不快点找<t color="red">沃玛角</t>。',
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    dialog.post(uid, questPath, '很好啊！你有足够的资格修炼刺杀剑术。祝贺你啊！',
                    dialog.link('npc_pour_wine', '下一步'))
                end,

                -- @mugong_asword_complete_next1, set [505]
                npc_pour_wine = function(uid, value)
                    if not server.player.hasItem(uid, '沃玛角', 1) then
                        return
                    end

                    dialog.post(uid, questPath,
                    '好了，现在喝用沃玛角做成的<t color="red">\'战酒\'</t>。' ..
                    '这个酒以后将保护你的灵魂。' ..
                    '这是为了获得学习刺杀剑术资格的仪式。' ..
                    '你很想知道为什么一定要割沃玛角来吧？' ..
                    '这其中的理由是前辈故人在学习刺杀剑术的时候，第一次切割的东西是沃玛角。' ..
                    '没有什么特殊的理由。',
                    dialog.link(SYS_EXIT, '喝了战酒。'))

                    server.player.removeItem(uid, '沃玛角', 1)
                    server.player.addItem(uid, '战酒', 1)
                    server.quest.setState(questUID, {uid = uid, state = 'quest_got_wine'})
                end,
            }
        ]])
    end,

    -- [505]. @mugong_asword_complete_next2 turns you away while the 战酒 is still in your pack,
    -- so the last step is to go and drink it
    quest_got_wine = function(uid, args)
        setQuestDesp{uid=uid, '喝掉龙血先生给的战酒，然后回去找他。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_LABEL] = '领刺杀剑术秘籍',
                [SYS_ENTER] = function(uid, value)
                    -- checkitem 战酒 1, still carrying it means you have not drunk it
                    if server.player.hasItem(uid, '战酒', 1) then
                        dialog.post(uid, questPath, '喝了战酒才可以学习武功哟。',
                        dialog.link(SYS_EXIT, '结束'))
                        return
                    end

                    -- @mugong_asword_complete_next3
                    dialog.post(uid, questPath,
                    {
                        '为了学习刺杀剑术，请在沃玛神殿找到<t color="red">沃玛角</t>。',
                        '我用你找来的沃玛角制成<t color="red">战酒</t>，喝了这个酒后就可以学习刺杀剑术了。',
                    },
                    dialog.link(SYS_EXIT, '结束'))

                    server.player.addItem(uid, '刺杀剑术（秘籍）', 1)
                    server.player.deliverGold(uid, 25000)
                    server.player.addItem(uid, '龙骨戒指', 1)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- MonQuest/asword.txt, 沃玛战士 in 沃玛神殿 while [503] is set
local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    {
        monster  = '沃玛战士',
        map      = hornMaps,
        state    = SYS_ENTER,
        kills    = hornKills,
        once     = true,
        give     = hornName,
        setState = 'quest_got_horn',
        say      = "（现在把沃玛角送给'龙血先生'就可以修炼'刺杀剑术'……）",
    },
}

-- @mugong_asword, the entry he offers to anyone who has not started
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼刺杀剑术',

        -- legacy left @mugong_asword clickable forever and answered out of the flags, so this
        -- stays up before the quest and after it. while it runs the EPUID behavior installed by
        -- the FSM hides this entry, npchar drops an EPQST entry whose quest has one
        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [702] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                dialog.post(uid, questPath, '你不是已经收到书吗？那么你为什么还要索要？',
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            -- checkjob warrior
            if not server.player.hasJob(uid, '战士') then
                dialog.post(uid, questPath, '该武功不是其它职业的人很容易就熟练的武功，只有<t color="red">战士</t>才可以掌握。',
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            -- checkmagic 刺杀剑术
            if server.player.hasMagic(uid, magicName) then
                dialog.post(uid, questPath, '你不是已经掌握该武功吗？请到其它的地方搞恶做剧。我可没有那么好的性格。',
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            dialog.post(uid, questPath, '谁？嗯，还是稚气未退的战士嘛，有什么事情找我吗？噢！想学刺杀剑术是吗？',
            dialog.link('npc_level_check', '下一步'))
        end,

        -- @mugong_asword_next3, checklevel 19
        npc_level_check = function(uid, value)
            if server.player.getLevel(uid) < minQuestLevel then
                dialog.post(uid, questPath, string.format('如果想学刺杀剑术，请将武功级别提高到<t color="red">%d</t>。', minQuestLevel),
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            dialog.post(uid, questPath, '我最不喜欢话多。如果有什么事情，请简单扼要地说明！',
            dialog.link('npc_lore1', '想了解刺杀剑术。'))
        end,

        -- @mugong_asword_next5
        npc_lore1 = function(uid, value)
            dialog.post(uid, questPath,
            {
                '和先天就有特别出色能力的的魔法师和多才多能并受到尊敬的道士相比，被任何任选择的一介小兵的我们，当然看起来既不特别、也不华丽。因此人们把战士叫做只有块头和力量的傻瓜。',
                '为了消除这种偏见，很多前辈们创造了杰出的武功并不断地发展。这中间有超越了人们想象力可以称为艺术的武功。刺杀剑术就是这些武功中的一个。',
            },
            dialog.link('npc_lore2', '真的吗？如果那样，为什么该武功没有被人知晓？'))
        end,

        -- @mugong_asword_next6
        npc_lore2 = function(uid, value)
            dialog.post(uid, questPath, '这是当然了，这个武功是战士们历经长久岁月的各种曲折、自尊心、生命，在任何考验中都不屈服的灵魂。你认为这个武功可以随便传授给任何人？只秘密地传授给具有真正战士灵魂的人们。',
            dialog.link('npc_lore3', '请传授我这个武功吧！'))
        end,

        -- @mugong_asword_next7, the nerve test. answering wrong ends it here
        npc_lore3 = function(uid, value)
            dialog.post(uid, questPath,
            {
                '战士们的高级武功是利用内力的深奥武功。你认为一次都没有使用过内力的你可以突然学习这种武功吗？很明显要发生大事故的。运气不好是死亡，如果运气好半身不遂。我不想看到比我年轻的人先死的样子。',
                '这样还要学习该武功吗？',
            },
            {
                dialog.link('npc_brave', '呃，这种觉悟都没有如何修炼武功？'),
                dialog.link('npc_coward', '看起来，我还有些勉强。'),
            })
        end,

        -- @mugong_asword_next8_2, he throws you out and you can walk back in and try again
        npc_coward = function(uid, value)
            dialog.post(uid, questPath, '这种傻瓜家伙！由于害怕，就想放弃，你这样的家伙还叫做战士？胆量连手指甲下的指甲泥那么大都没有的家伙！！当场从这里消失。',
            dialog.link(SYS_EXIT, '结束'))
        end,

        -- @mugong_asword_next8_1
        npc_brave = function(uid, value)
            dialog.post(uid, questPath, '哈哈。一无所有的家伙胆量很大。好的！我将把刺杀剑术的武功传授给你。',
            dialog.link('npc_lore4', '下一步'))
        end,

        -- @mugong_asword_next9
        npc_lore4 = function(uid, value)
            dialog.post(uid, questPath,
            {
                '简单地说刺杀剑术就是利用内力，<t color="red">在很远的地方刺杀敌人的技术</t>。',
                '实际上说用风压控制敌人比说用内力更正确。同下面要学习的武功相比还是入门的武功。',
            },
            dialog.link('npc_send_for_horn', '下一步'))
        end,

        -- @mugong_asword_next10, set [503]
        npc_send_for_horn = function(uid, value)
            dialog.post(uid, questPath, '但是在学习刺杀剑术之前，你要做一件事情。不是很困难的事情。进入沃玛神殿取得<t color="red">沃玛角</t>即可。不要问理由，快去快回！我将等你回来。',
            dialog.link(SYS_EXIT, '结束'))

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])
