-- converted from Envir/QuestDiary/NQ_BASE/oma.txt
--
-- 比奇城城主 sends you to find out how the 半兽人 are raising skeleton soldiers. the way in is
-- a room in 半兽洞穴 sealed by magic, so the quest runs two threads: get the scholar 云发 out
-- of the caves so he can read the seal, and work the 半兽人 for the key
--
-- 王铁匠's stolen hammer -> 角笛 off a 半兽战士 -> the sealed room opens -> 半块不死牌 off the
-- 半兽勇士 -> 不死牌 off the 骷髅精灵
--
-- the sealed room was a legacy MapQuest (Na_RoomOmaWarrior.txt) that refused entry unless you
-- carried the 角笛, here it is a grid trigger on the entrance, see setupSealedRoom below

_G.minQuestLevel = 11
_G.prequestName  = '王大人任务'

_G.sealedRoomMap = '半兽洞穴1层_D001'
_G.sealedRoomX   = 303
_G.sealedRoomY   = 65

local mondrop = require('quest.include.mondrop')

-- the entrance to 半兽勇士洞, shut until the 角笛 is in hand
--
-- pass the state the player is allowed to walk in on and bounce off, that first bounce is
-- what tells them there is a sealed room at all
local function setupSealedRoom(uid, discoverState)
    setupMapGridTrigger(sealedRoomMap, sealedRoomX, sealedRoomY, uid,
    string.format([[ return %s, getUID() ]], discoverState and ('"' .. discoverState .. '"') or 'nil'),
    [[
        local discoverState, questUID = ...
        return function(uid, x, y)
            if server.player.hasItem(uid, '角笛', 1) then
                server.player.postString(uid, '你举起角笛，封住洞口的魔法应声散开了！')
                return true
            end

            server.player.postString(uid, '这间屋子被一道古怪的魔法锁住了，怎么也进不去……')
            if discoverState and (server.quest.getState(questUID, {uid=uid}) == discoverState) then
                server.quest.setState(questUID, {uid=uid, state='quest_locked_room'})
            end
            return false
        end
    ]])
end

setQuestFSMTable(
{
    -- go and look, the point of this stage is to walk into the sealed door
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '接受了比奇城城主的委托，去半兽洞穴(65:174)1层附近(303:65)调查半兽人的异常征兆。'}
        setupSealedRoom(uid, SYS_ENTER)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '半兽洞穴的调查',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有关于半兽洞穴的调查结果吗？</par>
                            <par>看出异常征兆的地方就是<t color="red">半兽洞穴(65 : 174)1层附近 (303 : 65)</t>，所以希望你在那儿附近仔细调查！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- the door is sealed, only 云发 can read that kind of magic and he has gone missing
    quest_locked_room = function(uid, args)
        setQuestDesp{uid=uid, '半兽洞穴里有间被魔法锁住的屋子，去边境城市找精通此道的学者云发吧。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '半兽洞穴的调查',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，找到了有可疑迹象的地方但是入口被堵住了啊！</par>
                            <par>虽然有精通这方面的学者，不过现在却不知他的行踪啊！ 不过尽管如此，还是去找一下<t color="red">边境城市</t>一个叫做<t color="red">云发</t>的人吧！我也不知他的行踪到底……？</par>
                            <par><event id="npc_ask_scholar">云发现在在哪儿呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_scholar = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>他本来在边境城市的。</par>
                            <par>不过据说后来失踪了。如果对天然洞穴最了解的他在的话，这事儿就好解决多了！可能他是有什么不能露面不能联络的事情……</par>
                            <par>你就按照他夫人的嘱咐去天然洞穴找一下吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听云发的下落',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊……您是来找云发的……</par>
                            <par>呜……呜……</par>
                            <par><event id="npc_ask_what">出什么事儿了吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_what = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>他自从去调查天然洞穴走后就直到现在还没有回来！已经过了说好回来的日子好几天了，连个信儿都没有，所以我每天都是惶惑不安的独自度过的……</par>
                            <par>呜呜……真担心他出了什么事儿，实在是坚持不住了啊！</par>
                            <par>您一定要去把我在<t color="red">天然洞穴</t>的丈夫平安的找回来啊！如果您遇到了我丈夫的话，可能您就会知道您想要知道的事情的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_scholar'})
                end,
            }
        ]])
    end,

    -- 云发 is stuck in 天然洞穴2层 without his bodyguard
    quest_find_scholar = function(uid, args)
        setQuestDesp{uid=uid, '答应智善去天然洞穴找回她的丈夫云发。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听云发的下落',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊！老天爷啊……到底把我的丈夫……</par>
                            <par>啊…… 只能拜托<t color="red">%s</t>您了……</par>
                            <par>如果您能把他平安的带回来我一定会好好报答您的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('天然洞穴2层_D012_001', '云发_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和云发说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>什么？你说我的老婆十分担心我？</par>
                            <par>可这用魔法锁住的屋子…现在不是顾得上这个的时候……</par>
                            <par>嗯，那么就不该在这儿这样费时间了啊！</par>
                            <par><event id="npc_go_back">那就赶快回去吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_go_back = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么应该快去找我的助手！虽然不久前我与他离散了，但他是个责任感很强的人，所以如果找不到我的话是不会出去的！</par>
                            <par>因为我一点儿武功都不会，所以一下子不能从这个地方出去……你替我去找找他行吗？我的助手可能在<t color="red">半兽天然洞穴 2层</t>的什么地方。</par>
                            <par><event id="npc_find_guard">好的，我去找一下试试！</event></par>
                        </layout>
                    ]=])
                end,

                npc_find_guard = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那就拜托啦！</par>
                            <par>遇到他的话就告诉他我在这儿！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_guard'})
                end,
            }
        ]])
    end,

    -- the bodyguard 嘉登 is still combing the caves for him
    quest_find_guard = function(uid, args)
        setQuestDesp{uid=uid, '云发要你去半兽天然洞穴找他失散的护卫武士嘉登。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '云发的消息',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>遇到我的丈夫了？啊！太感谢了！</par>
                            <par>他一切平安啊，他要找的助手就是护卫武士<t color="red">嘉登</t>先生啊！</par>
                            <par>快去找嘉登先生吧！洞中没有护卫武士只有他自己……这可不行啊！</par>
                            <par>快快去找嘉登先生吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('天然洞穴2层_D012_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和云发说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是被魔法锁住的屋子……</par>
                            <par>这下又没辙了！现在要拜托你一下，快去找我的助手告诉他我在这儿！</par>
                            <par>我的助手可能在<t color="red">半兽天然洞穴2层</t>的什么地方！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002', '嘉登_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '告诉嘉登云发的下落',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我是来这里调查矿产分布的学者的助手兼护卫武士。</par>
                            <par>但是由于应付怪兽的袭击把学者先生给丢了！</par>
                            <par>到底在哪呢？……千万不要出什么事儿啊！</par>
                            <par><event id="npc_tell_found">云发先生就在天然洞穴2层，他平安无事！</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_found = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>云发先生平安无事啊！担心死我了，真是万幸啊！</par>
                            <par>这儿比较安全，我马上就去把他接过来！</par>
                            <par><event id="npc_guard_go">云发先生也正在等着你呢！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_go = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>云发先生平安无事，这真是万幸啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_scholar_safe'})
                end,
            }
        ]])
    end,

    -- reunited under 嘉登's guard, 云发 can finally look at the seal
    quest_scholar_safe = function(uid, args)
        setQuestDesp{uid=uid, '嘉登已经把云发接到安全的地方，去找云发问问那间被魔法锁住的屋子。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '云发的消息',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>顺利见到嘉登先生了啊！</par>
                            <par>谢谢……但是……啊…他要完成研究啊！</par>
                            <par>那我就只能耐心等他回来了！不管怎么样既然知道他一切都好，我就放心多了！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002', '嘉登_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和嘉登说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>云发先生平安无事，这真是万幸啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问被魔法锁住的屋子',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好不容易一行人又聚到一起了啊！谢谢啦！</par>
                            <par>在半兽洞穴被魔法锁住的屋子……</par>
                            <par>唔……魔法……半兽人……幸亏是和我最近研究的领域差不多的……</par>
                            <par><event id="npc_ask_reason">知道为什么进不去的理由了吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_reason = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唔……看来使半兽人的骷髅复活的魔法和这个地方有着某种联系的可能性确实很大啊！</par>
                            <par>那么可能半兽人有自己进入这个地方的办法。是固定的暗语还是特别的钥匙呢……？可能是要有特别的钥匙才行。</par>
                            <par>或者…能够进入这个地方的关键好像还是在半兽人那儿……不知您能不能去<t color="red">比奇城城主</t>大人那儿问问有没有半兽人的异常征兆！还有，我还要继续留在这儿， 如果还有什么要问的事儿的话，就来有嘉登看守的这里找我吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_ask_key'})
                end,
            }
        ]])
    end,

    -- the key is on the 半兽人 themselves, 城主 sends you to the camp by the caves
    quest_ask_key = function(uid, args)
        setQuestDesp{uid=uid, '云发说进屋子要一把特别的钥匙，去问比奇城城主半兽人最近的异常征兆。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问被魔法锁住的屋子',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>或者…能够进入这个地方的关键好像还是在半兽人那儿。不知道带着这种东西的半兽人在其他的地区会不会有什么动静，去比奇城城主大人那儿问问吧！</par>
                            <par>还有，我还要继续留在这儿， 如果还有什么要问的事儿的话，就来有嘉登看守的这里找我吧！唉……真想快点结束调查回到我妻子身边啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '禀报云发的推断',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>遇到云发了！ 那么就去他那儿问一下半兽洞穴的困魔咒是怎么回事儿吧！</par>
                            <par><event id="npc_report_seal">云发说要一把特别的钥匙才行。</event></par>
                        </layout>
                    ]=])
                end,

                npc_report_seal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯……让半兽人的骷髅复活的魔法和那个地方有着某种关联的可能性看来很大啊！</par>
                            <par>这样啊……我觉得非同寻常啊，到底是暗号呢…还是特别的钥匙…半兽人…半兽人的…那么还是去道观周围探查一下吧！既然最近那儿半兽人的活动频繁…没准就有知道什么线索的人呢！</par>
                            <par><event id="npc_ask_who">该找谁打听呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_who = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>听说最近<t color="red">华玉</t>很不安的样子。去帮帮她没准能得到有关特别的钥匙的情报呢！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_meet_smith'})
                end,
            }
        ]])
    end,

    -- 华玉 has nothing yet, but 王铁匠 next door has lost his hammer to them
    quest_meet_smith = function(uid, args)
        setQuestDesp{uid=uid, '比奇城城主让你去道馆一带找华玉打听半兽人的线索。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('道馆_1', '华玉_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听半兽人的异动',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>虽然为了以在这儿打猎的人为对象做生意，才来到这儿定居。不过虽然利益大，冒的风险也大啊！</par>
                            <par>原本遭到官兵讨伐削减了势力消失踪影的半兽人最近推举出了个厉害的半兽勇士，又有组织的聚集到了一起。这里已经受过它们不知多少次的肆虐践踏了！</par>
                            <par>如果一直是这样的话我也就该关门大吉，回乡养老去了！</par>
                            <par><event id="npc_ask_special">真是让你受苦了，可是或许您知道……</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_special = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你问半兽人有没有拿着什么特别的东西……？</par>
                            <par>嗯……这个, 最近好像没有什么特别的啊……</par>
                            <par>不过跟这个比起来，最近<t color="red">王铁匠</t>好像遇到了不少麻烦，你有什么帮助他的办法吗？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_hammer'})
                end,
            }
        ]])
    end,

    -- the hammer is on one of the 半兽人
    quest_find_hammer = function(uid, args)
        setQuestDesp{uid=uid, '王铁匠的铁锤被半兽人抢走了，去猎杀半兽洞穴的半兽人把铁锤找回来吧。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('道馆_1', '华玉_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和华玉说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>半兽人拿着的特别的东西……唔……好像没有什么啊……</par>
                            <par>不过最近看到王铁匠由于丢了铁锤十分伤心的样子，我就感到十分心疼啊……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '王铁匠_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于被抢走的铁锤',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你可能已经听华玉那儿听说了吧，由于遭受了好几次洗劫，现在店里已经没什么可用的东西了！</par>
                            <par>如果是其他的东西也就算了，但可一定要找回我的铁锤才行啊！你有所不知，对于我们铁匠来说，铁锤就是和性命一样重要的东西啊！况且那还是用千年木制成的珍品呢！</par>
                            <par><event id="npc_hear_hammer">竟有这样的事儿！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hear_hammer = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>所以就拜托您了！这附近的混蛋<t color="red">半兽人</t>中应该有一个拿着我铁锤的家伙，请你去帮我找回来吧！</par>
                            <par>我一定会报答你的！</par>
                            <par><event id="npc_ask_where">我的铁锤会在哪儿呢？</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_ask_where = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我的铁锤可能被这附近的其中一个半兽人拿着呢……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- hand the hammer back, 王铁匠 pays with what little he has left
    quest_got_hammer = function(uid, args)
        setQuestDesp{uid=uid, '找回了王铁匠的铁锤，快给他送回去吧。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('道馆_1', '王铁匠_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '归还王铁匠的铁锤',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>把我的铁锤找回来了啊！太谢谢你啦！这个铁锤被半兽人抢走后不知道我有多么伤心呢……</par>
                            <par>可是……虽然您这么辛苦帮我回了铁锤，却不知哪一天就又会被抢走！可能那时候我连命都也会搭进去呢！只有能够除掉那个<t color="red">半兽勇士</t>，让半兽人像以前那样不敢为非作歹，这事情才算完全的解决了啊……唉！</par>
                            <par><event id="npc_hand_hammer">我会做到的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_hammer = function(uid, args)
                    if not server.player.hasItem(uid, '王铁匠的铁锤', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我的铁锤可能被这附近的其中一个半兽人拿着呢……</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不管怎么样，这个是我对你帮我找回铁锤的报答！</par>
                            <par>我身上只有这个了，不好意思！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '王铁匠的铁锤', 1)
                    server.player.addItem(uid, '青铜斧', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_find_horn'})
                end,
            }
        ]])
    end,

    -- now 华玉 remembers the horn round a 半兽战士's neck
    quest_find_horn = function(uid, args)
        setQuestDesp{uid=uid, '帮了王铁匠之后，再去问问华玉半兽人身上那件奇怪的东西。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('道馆_1', '华玉_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '再问华玉半兽人的怪东西',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你问半兽人有没有拿着的什么特别的东西?</par>
                            <par>这个…谁知道呢……让我想想……哦，对了，好像不久前看见过一个<t color="red">半兽人战士</t>脖子上带了一个样子很奇怪的<t color="red">角笛</t>。虽然一次都没有听见过那半兽人吹笛子的声音，但是那个东西好像有点奇怪啊！</par>
                            <par>不过我也不知道那个半兽勇士在哪儿，可能在某个洞穴里藏着吧……</par>
                            <par><event id="npc_hunt_horn">我去找找那个角笛！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hunt_horn = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去找找那个半兽人战士带着的角笛怎么样？</par>
                            <par>由于有那个半兽勇士我也很忧心啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_hunt_horn'})
                end,
            }
        ]])
    end,

    -- the horn comes off a 半兽战士
    quest_hunt_horn = function(uid, args)
        setQuestDesp{uid=uid, '猎杀半兽战士，夺取它们脖子上那个奇怪的角笛。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('道馆_1', '华玉_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和华玉说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉！真是太吃力了，快要挺不住了……</par>
                            <par>唔…跟你这样的人诉苦也没什么用……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和智善说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这个人继续工作的时候我也不能放心的睡大觉啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '禀报角笛的线索',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>角笛……一定有什么特别的联系，去找找吧～！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '告诉云发角笛的事',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>角笛……难道是角笛？去找找吧！显然是有关系的东西！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- with the horn the seal opens, and the 半兽勇士 is waiting behind it
    quest_hunt_warrior = function(uid, args)
        setQuestDesp{uid=uid, '拿到了角笛，带着它去半兽洞穴1层(303:65)打开那间屋子，除掉半兽勇士。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '禀报角笛的线索',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>找到了角笛……既然叫做角笛……那么赶快去半兽洞穴二层调查一下那个奇怪的屋子吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '告诉云发角笛的事',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>果然是角笛呀……不管怎么样真的要出大问题了……快去半兽洞穴调查一下吧！</par>
                            <par>显然是通过魔法才能做到的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- half the token is in hand, the 城主 wants 云发 brought in to read it
    quest_return_token = function(uid, args)
        setQuestDesp{uid=uid, '从半兽勇士身上得到了半块不死牌，拿回比奇省内城交给比奇城城主。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '给云发看半块不死牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊……虽然不希望是那样……到底是不死牌啊！快去比奇城城主大人那儿看看吧！</par>
                            <par>虽然我也想跑去，可我这身子骨儿……去不了啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '呈上半块不死牌',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '半块不死牌', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>还没有关于半兽洞穴的调查结果吗？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>噢？你说这就是藏着那恶毒的魔法秘密的东西？</par>
                            <par>我果然没有看错人啊！真是太辛苦了！现在终于可以阻止半兽人集结不死的军队来进攻比奇省的事态了！</par>
                            <par>可是……现在还是没法儿安心下来！因为我们现在还没有对这个来历不明的魔法有完全的了解，万一什么时候这些半兽人再次策划这样的阴谋呢？还是这次就斩草除根为妙啊！</par>
                            <par>所以还是再拜托你一件事儿，去找到<t color="red">云发</t>把他带到我这儿来。因为这件事看起来是个非比寻常的问题，所以让他马上放下手中的事儿跟你来我这儿！</par>
                            <par><event id="npc_hand_half">剩下的事儿我跟先生分析分析这个东西后再决定吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_half = function(uid, args)
                    if not server.player.hasItem(uid, '半块不死牌', 1) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快去把云发先生带来！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '半块不死牌', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_fetch_scholar'})
                end,
            }
        ]])
    end,

    -- fetch 云发 out of the caves
    quest_fetch_scholar = function(uid, args)
        setQuestDesp{uid=uid, '比奇城城主要你把云发请回比奇省来一起分析不死牌。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于云发',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快去把云发先生带来！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002_001', '云发_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '请云发回比奇省',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>要我中断调查马上回来？</par>
                            <par>明白了，那么我现在马上回比奇省去。</par>
                            <par><event id="npc_scholar_home">那么比奇省再会！</event></par>
                        </layout>
                    ]=])
                end,

                npc_scholar_home = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好，后会有期！</par>
                            <par>请马上出发， 我也马上要用卷离开了</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_scholar_home'})
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽天然洞穴_E002', '嘉登_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和嘉登说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我也打算马上回比奇省。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- 城主 pays for the 半兽勇士 job and points you at 云发's analysis
    quest_scholar_home = function(uid, args)
        setQuestDesp{uid=uid, '云发已经动身回比奇省，去向比奇城城主复命吧。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '云发的消息',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您说比奇城城主大人下令让云发回来？</par>
                            <par>现在马上就能见到云发了！谢谢你告诉我啊……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '向比奇城城主复命',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是太辛苦您啦！</par>
                            <par>因为您事情办得很好，所以一切都会进展的很顺利的！</par>
                            <par>这是给你的报答，请收下吧！</par>
                            <par><event id="npc_take_reward">这只是我应该做的事儿啊…… 可是云发先生……?</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_reward = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>云发很担心自己的妻子就先回家去了！</par>
                            <par>他说用自己家的设备来分析一下不死牌，不知道现在有没有什么结果了。你去<t color="red">边境城市</t>的云发先生家里打听一下有什么分析结果吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '回城卷', 6)
                    server.quest.setState(questUID, {uid=uid, state='quest_ask_analysis'})
                end,
            }
        ]])
    end,

    -- 云发 is home, and the other half of the token is on the 骷髅精灵
    quest_ask_analysis = function(uid, args)
        setQuestDesp{uid=uid, '去边境城市云发先生家里问问不死牌的分析结果。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于不死牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在可能分析完不死牌了吧！你去趟边境城市跟云发先生打听一下有什么结果吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('志善屋_01_001', '云发_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问不死牌的分析结果',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>首先感谢您帮了我妻子的忙！</par>
                            <par>要是没有您的话！我到现在还在天然洞穴中进退两难，让妻子为如此我担心。</par>
                            <par>哦，对了，对不死牌的分析刚刚结束！这其实是古代的遗物，而并非是半兽人所制，只是偶然间流落到了半兽人手中！半兽人那样的家伙是造不出这样的东西的……</par>
                            <par><event id="npc_ask_more">原来如此啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_more = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>所以只要把这个东西毁掉的话，半兽人就无法使用魔法了。不过这不死牌原本是由两半合二为一组成的，现在我们只有这半块儿，所以一定要找到那另外的半块儿！</par>
                            <par>我觉得这所有的事情的幕后都是由那个最近聚集势力的半兽勇士搞得鬼。您有找到这剩下的半块儿不死牌的力量，那么一定能找到那个半兽勇士。</par>
                            <par>关于半兽勇士的情报请去告诉比奇城城主大人。我来这里的时候收集了各处的情报，现在可以掌握大概的位置了。</par>
                            <par><event id="npc_take_half">知道了！我会在比奇省转告这些的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_half = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那就再多辛苦一下啦！</par>
                            <par>骷髅精灵好像还拿着剩下的那半块儿不死牌！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '半块不死牌', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_tell_lord'})
                end,
            }
        ]])
    end,

    -- 城主 marks the 骷髅精灵's lair
    quest_tell_lord = function(uid, args)
        setQuestDesp{uid=uid, '云发说剩下的半块不死牌在骷髅精灵手里，回去转告比奇城城主。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '转告云发的分析结果',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>果然！这一系列的事件都和幕后的骷髅精灵有关！那么去除掉骷髅精灵，再找到剩下的半块不死牌毁掉它的话，所有的事情就可以结束了！</par>
                            <par>好吧！那么再拜托您去办点儿事！不管怎么样还是让你这样手腕高明智勇双全的人把这件事儿全部办妥了我才能安心啊～！</par>
                            <par>如果你能除掉骷髅精灵的话我会额外重赏你的！，一定要让骷髅精灵最后死在不是您的部下，而是<t color="red">%s</t>您的手中才行啊！</par>
                            <par>如果这阵子的情报准确的话，那么骷髅精灵的藏身之所必然是在<t color="red">半兽洞穴2层(255 : 281)</t>的某个地方。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_hunt_spirit'})
                end,
            }
        ]])
    end,

    -- the 骷髅精灵 carries the other half
    quest_hunt_spirit = function(uid, args)
        setQuestDesp{uid=uid, '带着半块不死牌，去半兽洞穴2层(255:281)亲手除掉骷髅精灵。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于骷髅精灵',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>骷髅精灵显然在<t color="red">半兽洞穴2层 (255 : 281)</t>的什么地方……</par>
                            <par>一定要让骷髅精灵最后死在不是您的部下，而是<t color="red">%s</t>您的手中才行啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)
                end,
            }
        ]])
    end,

    -- 不死牌 whole at last, both 城主 and 智善 pay out
    quest_final = function(uid, args)
        setQuestDesp{uid=uid, '骷髅精灵已除，不死牌也凑齐了，回比奇城城主那儿复命，别忘了去边境城市看看智善。'}
        setupSealedRoom(uid, nil)

        setupNPCQuestBehavior('志善屋_01_001', '云发_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和云发说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>关于不死牌的处理问题现在正在和边境城市的云发先生商议之中。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '智善_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '智善的谢礼',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>欢迎光临！</par>
                            <par>多亏您帮忙我的丈夫才能平安无事的回来！</par>
                            <par>真心的感谢您！不知该怎么报答您的大恩大德……</par>
                            <par><event id="npc_take_gift">我也很高兴能帮助你！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_gift = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>虽然不是什么贵重的东西，但是代表了我的一片心意！</par>
                            <par>请您一定要收下啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '浪雨刀', 1)
                    server.player.addItem(uid, '强效太阳水', 5)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '呈交不死牌',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '不死牌', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你说处死了骷髅精灵，可是却没有找到不死牌？</par>
                                <par>这可真是奇怪了啊……</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>噢！这就是不死牌的真面目啊！</par>
                            <par>那么骷髅精灵呢……？啊，是嘛？没有了骷髅精灵，那么那些半兽人现在也就树倒猢狲散了！</par>
                            <par>您可真是为这比奇省做了一件大事儿啊！我代表朝廷和比奇省百姓对你深表谢意啊！</par>
                            <par>这是对您的辛苦表示一点谢意的东西，请你务必收下吧。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '不死牌', 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 30000)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

-- the whole trail of quest items, none of them are in the ordinary monster drop table
mondrop.setDropOnKill
{
    {
        monster  = '半兽人',
        state    = 'quest_find_hammer',
        kills    = 10,
        give     = '王铁匠的铁锤',
        setState = 'quest_got_hammer',
        say      = '（这好像就是铁匠被抢走的锤子）',
    },

    {
        monster  = '半兽战士',
        state    = 'quest_hunt_horn',
        kills    = 5,
        give     = '角笛',
        setState = 'quest_hunt_warrior',
        say      = '（虽然不知道是什么动物的犄角制成的这分明不是一件寻常的东西）',
    },

    -- the horn is what got you into the sealed room and it is spent bringing him down
    {
        monster  = '半兽勇士',
        state    = 'quest_hunt_warrior',
        need     = '角笛',
        take     = '角笛',
        give     = {{'半块不死牌', 1}, {SYS_GOLDNAME, 5000}},
        setState = 'quest_return_token',
        say      = '（艰难的战斗。 无论如何，我们掌握了能使沃玛遗骨复活的魔法的精华部分）',
    },

    {
        monster  = '骷髅精灵',
        state    = 'quest_hunt_spirit',
        need     = '半块不死牌',
        take     = '半块不死牌',
        give     = {{'不死牌', 1}, {'诅咒骷髅精灵头盔', 1}, {SYS_GOLDNAME, 7000}},
        setState = 'quest_final',
        say      = '（这个巨大的红骷髅到底是什么？估计半兽人暂时不具有威胁比奇省的实力。）',
    },
}

uidRemoteCall(getNPCharUID('比奇县_0', '比奇城城主_1'), getUID(), getQuestName(), minQuestLevel, prequestName,
[[
    local questUID, questName, minQuestLevel, prequestName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if server.player.getQuestState(uid, prequestName) ~= SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>本官的事儿，可不是随便什么人都能插手的。</par>
                        <par>先去把<t color="red">王大人</t>的事情办妥了再来见我吧！</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>好像你就是帮助王大人解决了他的事情的那个人吧！</par>
                        <par>不过你的等级还没有超过<t color="red">%d级</t>，很难帮助本官我啊！</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>您不就是帮助王大人解决了他的事情的那个人吗？</par>
                    <par><event id="npc_confirm_self">正是在下。</event></par>
                </layout>
            ]=])
        end,

        npc_confirm_self = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那太好了！我正好在找像您这样手腕高明的人呢</par>
                    <par>我想要拜托您去调查一下和最近比奇省周围蓄积势力的半兽人有关的事儿，你有接受的勇气吗？</par>
                    <par><event id="npc_accept_quest">好啊！我一定全力以赴。</event></par>
                    <par><event id="npc_ask_detail">我想先听听详细的事情。</event></par>
                </layout>
            ]=])
        end,

        npc_ask_detail = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>好啊！那我就从头开始给你讲讲吧！</par>
                    <par>我们的祖先是很久以前为了把侵袭我们人类的半兽人斩草除根，曾经向半兽人的根据地派遣出过的远征队。但是击溃了半兽人之后，由于发生了大地震失去了返回故国的路，于是就留在了比奇这个地方。此后，半兽人就和我们人类结下了代代的冤仇。</par>
                    <par>最近总是传来半兽人忽然集结势力在筹划着不寻常的阴谋的消息。大概好像是半兽人正在为了进攻比奇省，在已经死去了的自己同族的骷髅上施下魔法造出骷髅兵士。</par>
                    <par><event id="npc_accept_quest">明白了。这件事儿就交给我吧！</event></par>
                    <par><event id="npc_need_time">给我点时间考虑一下吧！</event></par>
                </layout>
            ]=])
        end,

        npc_need_time = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那么你就考虑一下再来吧！</par>
                    <par>不过事态好像比我们想象的还要严重，刻不容缓啊！</par>
                    <par>所以要尽快决定啊！</par>
                    <par><event id="npc_reconsider">我考虑好了。</event></par>
                    <par><event id="npc_take_longer">再给我一点儿考虑的时间吧！</event></par>
                </layout>
            ]=])
        end,

        -- the legacy [172] branch, you went away to think and came back
        npc_reconsider = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>决定了吗？</par>
                    <par>现在很难找到像你这样合适的人选去做这件事儿啊～！</par>
                    <par><event id="npc_accept_brief">明白了。这件事儿就交给我吧！</event></par>
                    <par><event id="npc_take_longer">再给我一点儿考虑的时间吧！</event></par>
                </layout>
            ]=])
        end,

        npc_take_longer = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>慎重考虑一下吧，不过可不要拖得太久哦！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept_brief = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>我要你做的事情很简单。</par>
                    <par>就是去调查半兽洞穴和它们制造骷髅兵士的魔法，如果可能的话就把它毁掉或者拿到这儿来！目前根据调查能够看到异常征兆的地方就是<t color="red">半兽洞穴(65 : 174)1层附近 (303 : 65)</t>，所以希望你在那儿附近仔细调查！</par>
                    <par>祝你成功啊！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_accept_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>虽然已经派出了好多人，但大部分不是毫无所得而归，就是有去无回啊！不过我觉得像您这样智勇双全手腕高明的人一定可以把这件事情做好！</par>
                    <par>本官要你做的事情很简单，就是去调查半兽洞穴和它们制造骷髅兵士的魔法，如果可能的话就把它毁掉或者拿到这儿来！目前根据调查能够看到异常征兆的地方就是<t color="red">半兽洞穴(65 : 174)1层附近 (303 : 65)</t>，所以希望你在那儿附近仔细调查！</par>
                    <par>祝你成功啊！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
