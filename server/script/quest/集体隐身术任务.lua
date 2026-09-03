-- converted from Envir/QuestDiary/MU_taoist/masshiden.txt
-- with its monster hook, MonQuest/masshiden.txt, registered in Envir/MapQuest.txt against
-- 沃玛战将 dying on the six 沃玛神殿 maps
--
-- the only skill quest that starts somewhere other than a teacher. the 杂货商 in 比奇县 repeats
-- what a runaway calls himself a 小贩 says about taoists — that one abandoned him to a pack of
-- monsters — and once you have heard it out, 清明子 sends you to 沃玛神殿2层 to find out what
-- really happened to 成致
--
-- the journal you bring back turns out to hold a botched reading of 集体隐身术, which is what
-- killed him, and that is the 秘籍 you get
--
-- flags: [723] done, [512] heard the rumour, [513] sent to investigate, [514] journal found
--
-- two names the legacy data is loose about, both kept as written: the 杂货商 is only ever
-- called 小贩 in dialogue, and the monster hook tells you to take the journal to 大飞圣僧 while
-- 清明子 is who actually holds the quest

_G.minQuestLevel = 23

_G.magicName = '集体隐身术'
_G.mijiName  = '集体隐身术（秘籍）'
_G.logName   = '成致日志'

_G.grocerMap = '比奇县_0'
_G.grocerNPC = '杂货商_1'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

-- random 20
_G.logChance = 20

_G.logMaps =
{
    '沃玛神殿1层_D022',
    '沃玛神殿2层_D023',
    '沃玛神殿1层_D032',
    '沃玛神殿2层_D033',
    '沃玛神殿1层_D042',
    '沃玛神殿2层_D043',
}

setQuestFSMTable(
{
    -- set [512], the 杂货商 has told you the story and pointed you at 清明子
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '杂货商说的成致的事，去道馆本馆问清明子。'}

        -- the [512] and not [513] branch of @mugong_masshiding_pre, he nags you to go
        setupNPCQuestBehavior(grocerMap, grocerNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '成致的事',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有拜见清明子吗？</par>
                            <par>去了以后问一下叫<t color="red">小贩</t>男人的情况。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        -- @mugong_masshiding0_1 onwards, 清明子's side of it
        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            -- @mugong_masshiding3, the description of the magic. he also gives it unprompted to
            -- anyone who has not heard the rumour, which is the ELSESAY of @mugong_masshiding0
            local blurb = [=[集体隐身术和隐身术相同的是可以隐藏自己的动静，不同的是<t color="red">集体隐身术可以隐藏包括你同事动静的魔法</t>。]=]

            return
            {
                [SYS_LABEL] = '问成致的事',
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呀，是你哟。今天有什么事情找我吗？</par>
                            <par></par>
                            <par><event id="npc_about_peddler">是的，今天是因为叫小贩人的事情。。。。</event></par>
                            <par><event id="npc_just_greet">也就是为了问安和咨询而来的。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding1_1
                npc_just_greet = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好的，我没有其它的事情。你也过得不错吧！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_masshiding1_2
                npc_about_peddler = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呀，知道那个故事吧。真是很焦急的事情。</par>
                            <par>叫成致的人不是被判朋友的人，不知道是怎么回事儿。一定有不得已的缘由吧！</par>
                            <par>实际上我也认为此事有些诧异，你听说了有关成致的其它事情吗?</par>
                            <par></par>
                            <par><event id="npc_heard_book">我听说他找到了集体隐身术的武功书。</event></par>
                            <par><event id="npc_heard_nothing">没有听说其它的事情。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding2
                npc_heard_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>什么叫集体隐身术。。你了解集体隐身术吗？</par>
                            <par></par>
                            <par><event id="npc_explain_magic">不了解， 请对集体隐身术进行一下说明。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding3
                npc_explain_magic = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你知道隐身术是隐藏行踪的魔法吧？</par>
                            <par>%s</par>
                            <par>如果掌握该魔法，你就可以给其他人更多的帮助。要学习集体隐身术吗？</par>
                            <par></par>
                            <par><event id="npc_want_learn">是的，要学习。</event></par>
                            <par><event id="npc_not_yet">不，下次机会吧...</event></par>
                        </layout>
                    ]=], blurb)
                end,

                -- @mugong_masshiding4, the route for someone who has not heard about the book
                npc_heard_nothing = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这样的。那么你可以对叫成致的人为什么行踪不明进行调查吗？</par>
                            <par></par>
                            <par><event id="npc_accept">好的，我要试一试。</event></par>
                            <par><event id="npc_not_yet">现在还有其它的事情。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding5
                npc_want_learn = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么，在学习集体隐身术之前先测试你是否有学习集体隐身术的资格。。</par>
                            <par></par>
                            <par><event id="npc_accept">好的，我将试一试。</event></par>
                            <par><event id="npc_not_yet">我现在还没有做好心理准备。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding6_1
                npc_not_yet = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道了。下次准备好了，再来！</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_masshiding6_2, set [513]
                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真不愧是特殊的年轻人，成致失踪的地点是<t color="red">沃玛神殿的2层</t>。</par>
                            <par>希望到那个地方找到他为什么失踪的<t color="red">头绪</t>。</par>
                            <par></par>
                            <par><event id="%s" close="1">好的，知道了。</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid = uid, state = 'quest_investigate'})
                end,
            }
        ]])
    end,

    -- [513], off to 沃玛神殿
    quest_investigate = function(uid, args)
        setQuestDesp{uid=uid, '去沃玛神殿打沃玛战将，找成致留下的东西。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '成致的调查',

                -- the [513] and not [514] branch of @mugong_masshiding
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很遗憾没有找到任何东西哟。还有想再调查的想法吗？</par>
                            <par>如果有想法，快点收集成致的<t color="red">信息</t>。</par>
                            <par></par>
                            <par><event id="npc_explain">这件事要怎么做？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                -- @mugong_masshiding_explain
                npc_explain = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果想学习集体隐身术，首先从商人那儿听取<t color="red">对某个男子的传闻</t>，然后来找我。</par>
                            <par>我将拜托你到沃玛神殿2层找到那个人的<t color="red">痕迹</t>，发现这个东西即可。</par>
                            <par>称为痕迹的东西有可能就是他的<t color="red">日志</t>。他的日志被那个地方的怪兽拿着的可能性很高。</par>
                            <par></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- [514], the journal is in your pack
    quest_got_log = function(uid, args)
        setQuestDesp{uid=uid, '找到成致日志了，回本馆交给清明子。'}

        setupNPCQuestBehavior(teacherMap, teacherNPC, uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交成致日志',
                [SYS_ENTER] = function(uid, value)
                    -- the ELSESAY of @mugong_masshiding_complete
                    if not server.player.hasItem(uid, '成致日志', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>这个人很辛苦找到的东西丢失在哪儿了？</par>
                                <par>快点拿去吧！</par>
                                <par></par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    -- @mugong_masshiding_complete1, take 成致日志 1
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，终于找到了<t color="red">日志</t>。用这个东西就可以发现道士失踪的原因哦。</par>
                            <par>(拿走日志后，看写的文章...)</par>
                            <par>嗯...</par>
                            <par>这个。因此他虽然将集体隐身术的武功书握在手里，却没有完全掌握的样子。</par>
                            <par></par>
                            <par><event id="npc_read_log">是集体隐身术的要诀？</event></par>
                        </layout>
                    ]=])

                    server.player.removeItem(uid, '成致日志', 1)
                end,

                -- @mugong_masshiding_complete2
                npc_read_log = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是这样的。日志中包含有<t color="red">集体隐身术的要诀</t>。</par>
                            <par>但是凭借这种错误分析的要诀如何可以学习到正宗的魔法。</par>
                            <par>看来成致实施了集体隐身术，却失败了。在只有隐藏自身的状态下，走火入魔而失去了生命哟。</par>
                            <par>珍贵的生命就这样消失了。。。</par>
                            <par>魔法就是这样可怕的哟。自己没有做好也有可能失去生命，那你以后还想继续学习魔法吗？</par>
                            <par></par>
                            <par><event id="npc_take_book">是的，我以后将继续学习魔法。</event></par>
                        </layout>
                    ]=])
                end,

                -- @mugong_masshiding_complete3
                npc_take_book = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道了。你的决心非常大嘛。</par>
                            <par>你已经在其它地方得到了武功秘籍，我也没有再给你的必要了。我给你一些金币和东西，用在需要的地方。</par>
                            <par>我将向小贩解释。</par>
                            <par></par>
                            <par><event id="%s" close="1">谢谢！</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '集体隐身术（秘籍）', 1)
                    server.player.addItem(uid, '暗黑竹笛', 1)
                    server.player.deliverGold(uid, 21000)
                    server.quest.setState(questUID, {uid = uid, state = SYS_DONE})
                end,
            }
        ]])
    end,
})

-- MonQuest/masshiden.txt, one 沃玛战将 in twenty while [513] is set
local mondrop = require('quest.include.mondrop')

mondrop.setDropOnKill
{
    {
        monster  = '沃玛战将',
        map      = logMaps,
        state    = 'quest_investigate',
        chance   = logChance,
        once     = true,
        give     = logName,
        setState = 'quest_got_log',
        say      = "（现在回到大飞圣僧那儿，并将 '成致日志'拿给他，就可以学习'集体隐身术'了...）",
    },
}

-- @mugong_masshiding_pre, the 杂货商's story. this is where the quest starts
uidRemoteCall(getNPCharUID(grocerMap, grocerNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '小贩男子的传闻',

        -- he only brings 成致 up with a taoist who could actually learn the magic, which is
        -- what @mugong_masshiding_pre1 and pre2 check before pre3 says anything
        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '道士') then
                return false
            end

            if server.player.getLevel(uid) < minQuestLevel then
                return false
            end

            if server.player.hasMagic(uid, magicName) then
                return false
            end

            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        -- @mugong_masshiding_pre3
        [SYS_ENTER] = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你看起来是非常有实力的道士哦。你知道有关<t color="red">小贩</t>男子的故事吗？</par>
                    <par></par>
                    <par><event id="npc_never_heard">没有听说过的名字...</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_masshiding_pre4
        npc_never_heard = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>哦，不知道也是理所当然的。这个男人是不久之前逃到比奇省的伪道士，到处讲道士们的坏话。</par>
                    <par>但是听了他的故事，他也很为难哟。</par>
                    <par></par>
                    <par><event id="npc_tell_more">可以讲一讲他有什么事情吗？</event></par>
                    <par><event id="npc_leave_early">我很忙，就到此为止要走了。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_masshiding_pre5_1
        npc_leave_early = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>好的，那么请走好。下次不要忘了多买些东西。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_masshiding_pre5_2
        npc_tell_more = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>他是边境城市出身的战士，和叫<t color="red">成致</t>的道士关系非常好。某个时候他们为了和怪兽战斗而出去了，但是一次聚集了很多的怪兽，他们处于危险的境地。</par>
                    <par>他们遇到了生死危机，那个道士偷偷隐藏自己的行踪不见了。那以后小贩总是批评道士们表里不一。</par>
                    <par>我所知道的成致决不是那样虚伪的人呀。。。</par>
                    <par>好像有什么误会。</par>
                    <par></par>
                    <par><event id="npc_want_more">嗯，很有趣儿，想多知道些儿。</event></par>
                    <par><event id="npc_not_my_problem">与我没有关系的事情哟。到此为止，我要走了。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_masshiding_pre6_1
        npc_not_my_problem = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>我讲话看起来很乏味，请走好！</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_masshiding_pre6_2, set [512]
        npc_want_more = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>对不起，更详细的内容我也不知道。</par>
                    <par>如果真的想知道，拜见<t color="red">清明子(道馆本馆,11:10)</t>如何？?</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid = uid, state = SYS_ENTER})
        end,
    })
]])

-- 清明子 answers about 集体隐身术 whether or not the 杂货商 has said anything, which is the
-- ELSESAY of @mugong_masshiding0, plus the two lines that only depend on the flags
uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName,
[[
    local questUID, questName, minQuestLevel, magicName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '问集体隐身术',

        [SYS_CHECKACTIVE] = function(uid)
            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [723] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书吗？那为什么还想索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 集体隐身术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>那么困难学到的集体隐身术正在灵活地使用吧？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 23
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>集体隐身术。。如果对这样的武功感兴趣，好像需要再修炼些。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- the ELSESAY of @mugong_masshiding0: without [512] he will describe the magic but
            -- has nothing to send you after, the 杂货商 has to bring 成致 up first
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>集体隐身术。。。</par>
                    <par>集体隐身术和隐身术相同的是可以隐藏自己的动静，不同的是<t color="red">集体隐身术可以隐藏包括你同事动静的魔法</t>。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])
