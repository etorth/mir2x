-- converted from Envir/QuestDiary/NQ_BASE/tarak.txt
--
-- the 不死牌 you brought back from the 半兽人 has been stolen out of the 衙门. the thief is
-- 署箭, a 道馆 apprentice who ran off with their old books years ago and has been raising the
-- dead with it ever since
--
-- you corner him three times: in the 连接通路, in the ruined mine, and finally in his lair
-- behind a 困魔咒 that only a charm made of 僧侣僵尸骨 and 雷电僵尸骨 can break
--
-- 署箭 never fights you himself, he sets his latest creation on you and slips away. legacy
-- despawned him and spawned a monster in his place; mir2x NPCs cannot despawn yet, so
-- npcbattle.turnHostile leaves him standing but takes his voice away until the quest gives it
-- back, see quest/include/npcbattle.lua
--
-- the final stand is at 废矿矿山地下2层_D404_002 rather than the legacy 毒蛇山谷 mine, because
-- that is where the second 署箭 NPC actually stands in this repo

_G.minQuestLevel = 16
_G.prequestName  = '被盗灵魂任务'

_G.tunnelMap  = '连接通路_E402_001'
_G.tunnelX    = 13
_G.tunnelY    = 10

_G.lairMap    = '废矿矿山地下2层_D404_002'
_G.lairX      = 22
_G.lairY      = 11

local mondrop   = require('quest.include.mondrop')
local npcbattle = require('quest.include.npcbattle')

setQuestFSMTable(
{
    -- the 城主 has just found the 不死牌 missing
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '不死牌被盗，比奇城城主要你去道馆找执客院住持书堂玄震道士打听盗贼的下落。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '被盗的不死牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不知盗走不死牌的犯人到底要做什么坏事儿，真担心啊！</par>
                            <par>去道馆见一下执客院住持<t color="red">书堂玄震</t>道士吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听偷走不死牌的道士',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>贫道乃执客院住持书堂玄震！</par>
                            <par>请问施主是为何事而来？</par>
                            <par><event id="npc_tell_theft">我是来找在比奇省衙门偷走不死牌的那个道士的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_theft = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦？竟有此等事情，施主可否详细道来呢？</par>
                            <par>.......</par>
                            <par>噢！听说半兽人以半兽勇士为中心集结势力，可突然间又安静了下来，我正感到十分奇怪呢，原来都是这位年轻的施主您解决的啊！真是了不起啊！</par>
                            <par>哦……那个带着灵魂护卫摄人魂魄进行研究的道士偷走了不死牌？</par>
                            <par>咳……这个家伙终于闹出了大事儿啊！</par>
                            <par><event id="npc_ask_who">他是谁呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_who = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>首先要说的是……现在我要给施主讲的故事是我们道馆的耻辱，原本不该跟外人说的…所以请您答应老纳，别随便把这些话故意传到其他不相关的人那儿！</par>
                            <par><event id="npc_promise">好的，我答应您！</event></par>
                        </layout>
                    ]=])
                end,

                npc_promise = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这是很久以前的事儿了！</par>
                            <par>有个聪明的年青人进入我们道馆门下。由于才能和悟性十分出众，所以得到了门派中元老们的特别器重。甚至到了打算让他做继任馆主的程度。可是谁想到那个家伙加入我们道门却另有居心！他对什么济世求道、上仙药手之类的东西毫不关心。最后他终于不顾禁令拿了几卷古书和灵魂护卫逃走了。因此本馆将他开除出门派并且下了追杀令，可是直到现在还没能找到他。那个堕落道士叫<t color="red">署箭</t>……这次的事儿一定是他搞得鬼！</par>
                            <par><event id="npc_ask_goal">那么他觊觎的是长生不老的力量……</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_goal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>就是啊！一心想要长生不老的这个家伙完全能做出这种事儿来。唉……本来应该是由本馆解决的事儿却引发了如此祸端，贫道真是惭愧至极啊！您回比奇省的时候请转告一下，现在本馆将会尽全力帮助解决这件事情的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_report_thief'})
                end,
            }
        ]])
    end,

    -- name in hand, the 城主 has a sighting
    quest_report_thief = function(uid, args)
        setQuestDesp{uid=uid, '道馆说盗贼是被逐出师门的堕落道士署箭，回去禀报比奇城城主。'}

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和书堂玄震说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>为了抓住署箭那个家伙，我们会紧密配合的！</par>
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
                [SYS_LABEL] = '禀报署箭的身份',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那个叫署箭的堕落道士做了这种坏事儿？</par>
                            <par>咳～呸……真气煞我也！</par>
                            <par><event id="npc_ask_why">为什么这样说呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_why = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>警戒令发出之后，报上来很多对于在外城地区举动异常的道士的通报。其中也有来自派遣去扫荡在您的活动之下被击溃的半兽人古墓中藏身的半兽人余党的部队的通报，那个部队的侦察兵在通往矿区的地下洞穴中遇到了和从未见过的怪物们在一起的道士。</par>
                            <par><event id="npc_ask_where">没有看到就在眼皮底下的犯人啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_where = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>通报来了还没多久，所以他还不会走的太远。所以就只好辛苦您出面来调查一下这件事儿了。</par>
                            <par>如果抓到那个堕落道士的话，您就酌情处理好了，不过请一定要把不死牌带到这来啊！</par>
                            <par>哦，对了！我告诉你一下这附近的地理位置吧！</par>
                            <par><event id="npc_ask_map">那就拜托了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_map = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我想上次你解决事情的时候就应该已经知道天然洞穴和半兽洞穴3层是互相连接着的了吧！</par>
                            <par>但是在半兽洞穴3层通向天然洞穴的不仅仅是半兽天然洞穴，还有另外的连接通路。</par>
                            <par>那个连接通路经过比奇省和周边的江水下面到达比奇省东北部的矿山。</par>
                            <par>通报中所说的目击那个异常道士的地方就在那个连接通路。</par>
                            <par><event id="npc_go_tunnel">哦！明白了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_go_tunnel = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>目击那个异常道士的地方就在半兽洞穴3层的<t color="red">连接通路2层(63 : 72)</t>内。</par>
                            <par>快点去吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_hunt_taoist'})
                end,
            }
        ]])
    end,

    -- first meeting, he sets his试作品 on you rather than fight
    quest_hunt_taoist = function(uid, args)
        setQuestDesp{uid=uid, '去半兽洞穴3层的连接通路2层(63:72)找出堕落道士署箭。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于署箭的下落',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>一定要找回不死牌啊！</par>
                            <par>目击那个异常道士的地方就在半兽洞穴3层的<t color="red">连接通路2层(63 : 72)</t>内。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(tunnelMap, '署箭_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和署箭对质',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>比我想象的来的还要早啊！</par>
                            <par><event id="npc_name_him">你就是署箭道士！</event></par>
                        </layout>
                    ]=])
                end,

                npc_name_him = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>竟然连这儿都知道~？真是像传闻中说的那样手腕高明啊！</par>
                            <par><event id="npc_demand">我是来取回被你偷走了的不死牌的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_demand = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哈哈哈！您可真是个奇怪的人啊!难道我会那么轻易的还给你？还是别痴心妄想了，赶快回去吧！</par>
                            <par><event id="npc_press">别说大话，赶快拿出来！</event></par>
                        </layout>
                    ]=])
                end,

                npc_press = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>反正在你手里也实现不了这个宝物的真正价值，难道不该到合适的主人手里吗？哈哈，这个世界上唯一有资格拥有不死牌这个宝物只有我……研究了一辈子长生不老的署箭！</par>
                            <par><event id="npc_no_more_talk">好像没有再说什么的必要了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_no_more_talk = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我也正好是这么想的！不过我想好像没有冒这个险跟你亲自动手的必要，哈哈……正好有了一个试验我刚研究出来的怪物的机会。你就和它比试一下吧！</par>
                            <par>你的对手就是它！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_fight_zombie'})
                end,
            }
        ]])
    end,

    -- he stops answering and a 僧侣僵尸 comes at you
    quest_fight_zombie = function(uid, args)
        setQuestDesp{uid=uid, '署箭放出了他研究出来的僵尸，打倒它！'}

        npcbattle.turnHostile
        {
            map     = tunnelMap,
            npc     = '署箭_1',
            uid     = uid,
            monster = '僧侣僵尸',
            x       = tunnelX,
            y       = tunnelY,
            say     = '署箭口中念念有词，一具僵尸从地里爬了出来，署箭再也不理会你了！',
        }
    end,

    -- beaten, he finds you interesting enough to talk to again, then slips away
    quest_taoist_fled = function(uid, args)
        setQuestDesp{uid=uid, '打倒了署箭的僵尸，再去和他说话。'}

        setupNPCQuestBehavior(tunnelMap, '署箭_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和署箭对质',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>果然实力非凡啊！现在我终于相信你能独自打败半兽勇士的事儿了！</par>
                            <par><event id="npc_ask_purpose">你偷走不死牌就只是为了制造这种怪物？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_purpose = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>只是……？这东西算得了什么？我已经超越了死亡。和神仙没什么区别了！现在我马上就能够长生不老，永葆青春的生活下去了。到那时我就能超越神仙了！怎么样？难道不认为我的梦想很不错吗？</par>
                            <par><event id="npc_refuse">你别痴心妄想了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_refuse = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很遗憾！那只是个我看看你实力到底如何的机会而已！看来你真的可以和我一起联手支配世界啊……</par>
                            <par><event id="npc_call_mad">真是彻底疯了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_call_mad = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这样啊！结果又回到了原点！我可不想再和你作为对手而浪费体力。就先走一步了……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    -- legacy threw the player back out of the tunnel as he left
                    server.player.spaceMove(uid, '连接通路_E402', 63, 72)
                    server.quest.setState(questUID, {uid=uid, state='quest_report_mine'})
                end,
            }
        ]])
    end,

    -- the 城主 has worse news: the mine is overrun by the same things
    quest_report_mine = function(uid, args)
        setQuestDesp{uid=uid, '署箭又跑了，回比奇省内城向比奇城城主复命。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '禀报连接通路的遭遇',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>回来了啊！找到那个堕落道士的行踪了吗？</par>
                            <par><event id="npc_report_escape">虽然在连接通路里碰到了他，可还是让他跑掉了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_report_escape = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉！真是太遗憾了！</par>
                            <par>不过，那个叫署箭的家伙真的指挥了一群怪物作为他的部下吗？</par>
                            <par><event id="npc_confirm_monster">是的！好像都是一些腐烂的尸体似的东西，是从来没有见过的怪物！</event></par>
                        </layout>
                    ]=])
                end,

                npc_confirm_monster = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>本官可能知道那些东西是什么……</par>
                            <par><event id="npc_ask_what">到底是什么东西呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_what = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您去追赶那个疯道士进入地下的时候，本官收到了来自矿山难以置信的通报。</par>
                            <par>唉……就在几天前，一群与您所见的怪物一样的怪物突然之间出现在矿山之中袭击了工人们。</par>
                            <par>因此现在矿山已经成为一片废墟，而且最可怕的是，受这些怪物袭击而死去的人，尸体又变成了和那些怪物一样的东西……</par>
                            <par><event id="npc_ask_how">这是怎么回事儿呢?</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_how = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>可能又是靠不死牌的力量搞得什么鬼吧！连半兽人那么低的智商都能造出骷髅兵士，何况是……</par>
                            <par>不管怎么样都不能再任这个堕落道士恣意胡为了！无论如何都要破坏他制造的阴谋，避免更多的惨剧继续发生！</par>
                            <par><event id="npc_go_mine">好的，我马上就去矿山调查一下！</event></par>
                        </layout>
                    ]=])
                end,

                npc_go_mine = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>根据观察结果的通报，矿山中的怪物一共有5种形态：恶哑僵尸、独角僵尸和蚀豸僵尸都是死去的尸体复活后变成的一般僵尸，这些僵尸反复杀死几遍都会复活。</par>
                            <par>此外，叫法老侣僵尸的怪物因为生前是僧侣，所以还穿着僧侣的服装。叫做老道僵尸的怪物因为生前是法神，所以会使用魔法进行攻击。</par>
                            <par>不过幸运的是这后两种僵尸被打死后就不会再复活了。</par>
                            <par><event id="npc_thanks_info">这真是很有用的情报啊！谢谢啦！</event></par>
                        </layout>
                    ]=])
                end,

                npc_thanks_info = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>朝廷软弱无力，即使发生了如此的大事都要依靠百姓支持，真是惭愧惭愧啊！</par>
                            <par>只能给你这些帮助，还请原谅啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_search_mine'})
                end,
            }
        ]])
    end,

    -- second meeting, deep in the ruined mine
    quest_search_mine = function(uid, args)
        setQuestDesp{uid=uid, '矿山已被僵尸占据，去废矿矿山地下2层深处找出署箭。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于矿山的惨剧',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>矿山中发生的惨剧导致无辜的百姓死伤无数！</par>
                            <par>一定要尽可能快的除掉那个恶道，一天都不能耽搁啦！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(lairMap, '署箭_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和署箭对质',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>又是你？</par>
                            <par><event id="npc_stop_him">你不能马上停止这邪恶勾当吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_stop_him = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好像你误会什么了吧……</par>
                            <par>不过反正像你这样的智商不高的人是理解不了的！呵呵呵，我是在建造一个没有痛苦没有战争的人间乐园啊！</par>
                            <par>不过你是没有机会在这个乐园生活了！因为你就要死在这儿了！</par>
                            <par><event id="npc_defy">哼！我可不怕你制造的怪物！</event></par>
                        </layout>
                    ]=])
                end,

                npc_defy = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵呵，你的实力的确超出我的预料！</par>
                            <par>所以我也为你特别准备了一下！这次不会让你失望的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_fight_king'})
                end,
            }
        ]])
    end,

    -- the 僵尸王 he has been bragging about
    quest_fight_king = function(uid, args)
        setQuestDesp{uid=uid, '署箭放出了他新造的僵尸王，打倒它！'}

        npcbattle.turnHostile
        {
            map     = lairMap,
            npc     = '署箭_1',
            uid     = uid,
            monster = '尸王',
            x       = lairX,
            y       = lairY,
            say     = '署箭召出了一头庞然大物，它腐烂的身躯几乎顶到了洞顶！',
        }
    end,

    -- he shrugs it off and leaves again
    quest_king_dead = function(uid, args)
        setQuestDesp{uid=uid, '打倒了僵尸王，再去和署箭说话。'}

        setupNPCQuestBehavior(lairMap, '署箭_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和署箭对质',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你这种傻瓜啊！你怎么总来妨碍我的研究？</par>
                            <par>难道不想在世外桃源生活吗？</par>
                            <par><event id="npc_wake_up">别说大话了，还是清醒一下吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_wake_up = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哼！再试验一下的话就能掌握长生不老的力量了，没必要冒着危险跟你这样的人打架而自找麻烦！</par>
                            <par>今天就到此为止吧！我先走一步了！</par>
                            <par><event id="%s" close="1">关闭</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.spaceMove(uid, '地下2层采矿所_D404', 53, 129)
                    server.quest.setState(questUID, {uid=uid, state='quest_report_king'})
                end,
            }
        ]])
    end,

    -- report the 僵尸王, the 城主 wants the 道馆 told
    quest_report_king = function(uid, args)
        setQuestDesp{uid=uid, '署箭又逃走了，回比奇省内城向比奇城城主复命。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '禀报矿山的遭遇',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去矿山的事儿怎么样了？</par>
                            <par><event id="npc_report_escape">虽然在矿山深处找到了署箭，可还是让他跑掉了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_report_escape = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那个堕落道士真是像泥鳅一样不容易逮啊！</par>
                            <par>下次一定应该就是他的死期了……</par>
                            <par><event id="npc_mention_king">还有件关于他做实验的事情……</event></par>
                        </layout>
                    ]=])
                end,

                npc_mention_king = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>什么？那个家伙又制造出了一个叫做“僵尸王”的新怪物？</par>
                            <par>嗯……从他这样不停的制造怪物，显然可以看出他心里的打算啊！</par>
                            <par><event id="npc_ask_plan">是什么打算呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_plan = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>以前你在连接通路里跟他碰面的时候，他不是说过要和您一起支配世界这样的大话嘛！</par>
                            <par>如果能把由拥有这样强大能力的怪物组成军队掌握在手中的话，就没有不能支配世界的理由了啊！</par>
                            <par>不管怎么样这可是个大事儿啊！</par>
                            <par><event id="npc_ask_next">现在该怎么办才好呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_next = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这次让这个家伙逃脱，估计短时间内他不会再轻易露面了。我们还是一边进行全面的警戒，一边准备合适的办法来对付他筹划的阴谋吧！</par>
                            <par>还有应该去和道馆联系一下，把最近发生的情况告诉他们，相信他们能够找出破解这种不死的魔法的方法。</par>
                            <par>因为我还要去动员官兵们，这件事就交给你去办吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_tell_temple'})
                end,
            }
        ]])
    end,

    -- pass the word to 书堂玄震
    quest_tell_temple = function(uid, args)
        setQuestDesp{uid=uid, '把比奇城城主的话转告给道馆的书堂玄震道士。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于道馆',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>把我的话转告给书堂玄震道士去吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '转告署箭的动向',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊！署箭那个混蛋家伙做出了这种坏事儿？</par>
                            <par><event id="npc_tell_hiding">那个家伙藏了起来，不知是不是又在准备挑出什么事端呢……</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_hiding = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>知道了！</par>
                            <par>我们会继续派人去寻找那家伙有可能的藏身之处，同时研究破解那个不死牌的方法。</par>
                            <par>如果再有什么动静的话请继续来告诉我们。</par>
                            <par>如果我们这边发现了什么线索也会马上跟比奇省联络的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_temple_pledge'})
                end,
            }
        ]])
    end,

    -- the 城主 pays for the work so far, then word comes from 毒蛇山谷
    quest_temple_pledge = function(uid, args)
        setQuestDesp{uid=uid, '道馆答应全力协助，回比奇省内城向比奇城城主复命。'}

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和书堂玄震说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>请去比奇省转告一下吧！</par>
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
                [SYS_LABEL] = '转告道馆的答复',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>得到了道馆方面将会尽全力来协力解决这件事儿的承诺，我也稍微能安下心来了！</par>
                            <par>看来寻找那个堕落道士行踪的时期要全力以赴，拼了命也要跟他周旋到底。</par>
                            <par>不管怎么样，您为解决因官府失误而丢失重要物品才引发的这件事情如此尽心尽力，我除了感谢的话之外都不知该说什么才好了！</par>
                            <par><event id="npc_take_reward">这只是我理所应当做的而已啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_reward = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这是本官替朝廷赐给你的，虽然微薄但却表达了我诚意，请不必推辞务必收下啊！</par>
                            <par>请在要紧的时候用吧！</par>
                            <par>能够有如此协助官府工作的百姓，本官真的很满足啊！</par>
                            <par><event id="npc_hear_valley">还有别的消息吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_hear_valley = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>毒蛇山谷矿山又出现了和那次一样的现象？让尸体复活而造出的怪物……</par>
                            <par>听说那个叫署箭的堕落道士一向十分精明，怎么却又用起已经失败的伎俩呢？真是难以理解啊！这次一定要做好万全的准备……</par>
                            <par>天才的头脑如果用到歪处也是十分可怕的啊！不管怎么样还是要充分的注意才是，你去打听一下署箭那个家伙这次又要搞什么名堂吧！</par>
                            <par>所谓要知己知彼嘛！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, SYS_GOLDNAME, 10000)
                    server.quest.setState(questUID, {uid=uid, state='quest_find_lair'})
                end,
            }
        ]])
    end,

    -- the old miner saw where they came from
    quest_find_lair = function(uid, args)
        setQuestDesp{uid=uid, '毒蛇山谷的矿山又出现了僵尸，去找蛇谷老矿夫打听。'}

        setupNPCQuestBehavior('毒蛇山谷_2', '蛇谷老矿夫_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听矿山的怪事',
                [SYS_ENTER] = function(uid, args)
                    -- he only warms to you once 珍珍 has been cured, that is 千年毒蛇任务
                    if server.player.getQuestState(uid, '千年毒蛇任务') ~= SYS_DONE then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>珍珍正处于死境之中啊！</par>
                                <par>善良可爱的孩子…唉……连金中医都无从下手……啧啧！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是治好珍珍的病的那个年轻人？</par>
                            <par>在这么危急的情况下有像您这样的人来到我们村子，真是万幸啊！</par>
                            <par><event id="npc_ask_matter">有什么事儿吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_matter = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>昨天身体状态还可以，就带着鹤嘴锄进了矿山。差不多采完了近处质量较好的矿石后又进入了矿坑深处。像往常一样用鹤嘴锄采矿的时候，却不知从哪儿传来了奇怪的声音！</par>
                            <par>最开始的时候没太在意，可是过了一会却有一股什么腐烂了的恶臭扑鼻而来！</par>
                            <par>看来的确有点什么不对劲儿，于是停下了手里的活儿进入矿山更深处一看！</par>
                            <par><event id="npc_ask_saw">腐烂的恶臭……啊……</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_saw = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>天哪！原来里面是有好多尸体在走来走去的！</par>
                            <par>那些家伙一看到我就发出了奇怪的声音向我扑了过来！有用独腿跳着来的，还有干脆爬着来的，还有闪着奇怪的火光的！</par>
                            <par>我扔下手中的鹤嘴锄拼了命才能逃了出来！</par>
                            <par>但是这种话谁都不会相信的，所以直到今天才第一次跟您说！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_report_lair'})
                end,
            }
        ]])
    end,

    -- the lair is sealed, the 城主 sends you back to the 道馆
    quest_report_lair = function(uid, args)
        setQuestDesp{uid=uid, '打听到了署箭的藏身之所，回去禀报比奇城城主。'}

        setupNPCQuestBehavior('毒蛇山谷_2', '蛇谷老矿夫_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和蛇谷老矿夫说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不死牌？署箭？你的话好像更是奇怪啊！</par>
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
                [SYS_LABEL] = '禀报署箭的藏身之所',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！你说已经找到了署箭那个家伙的藏身之所？终于知道了那个恶道的住处啦！</par>
                            <par>辛苦你了！不过那个家伙会用困魔咒，还是拿他无从下手啊！</par>
                            <par>好像还是需要他们的帮助才行啊！</par>
                            <par><event id="npc_ask_temple">您是说让我去道馆吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_temple = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是啊！去书堂玄震道士那儿问问是否有能够破解困魔咒的方法。</par>
                            <par>不管署箭那个家伙到底在那里面策划着什么阴谋，都一定要在他得逞之前阻止他啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_ask_seal'})
                end,
            }
        ]])
    end,

    -- the charm needs two kinds of bone
    quest_ask_seal = function(uid, args)
        setQuestDesp{uid=uid, '署箭的藏身之所被困魔咒封住，去问书堂玄震道士有没有破解的方法。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于困魔咒',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>对我们来说，已经没有余暇等到那个家伙做好充分的准备设下困魔咒出来的时候了！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问破解困魔咒的方法',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快请进，有什么新的消息吗？</par>
                            <par><event id="npc_tell_seal">找到了署箭的下落，可是那个地方由于用不死牌设下了困魔咒进不去啊</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_seal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是这样啊……以前施主能够破除困魔咒是因为有半块不死牌在身。</par>
                            <par>可是目前署箭那个家伙带着完整的不死牌呆在困魔咒里，如果不破解困魔咒的话是进不去的。</par>
                            <par>嗯……不过也不是没有办法的。</par>
                            <par><event id="npc_ask_way">是什么办法呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_way = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>贫道得知如果有矿山出现的怪物们中<t color="red">僧侣僵尸骨</t>和<t color="red">雷电僵尸骨</t>的话，或许可以造出破解署箭施下魔法的护身符来！</par>
                            <par>虽然很辛苦，但还是请您去矿山找僧侣僵尸骨和雷电僵尸骨来吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_collect_bones'})
                end,
            }
        ]])
    end,

    -- the bones come off the two zombies that stay dead
    quest_collect_bones = function(uid, args)
        setQuestDesp{uid=uid, '去矿山猎杀僧侣僵尸和雷电僵尸，取得僧侣僵尸骨和雷电僵尸骨。'}

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交出僵尸骨',
                [SYS_ENTER] = function(uid, args)
                    local hasMonkBone  = server.player.hasItem(uid, '僧侣僵尸骨', 1)
                    local hasThunkBone = server.player.hasItem(uid, '雷电僵尸骨', 1)

                    if not (hasMonkBone or hasThunkBone) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>需要僧侣僵尸骨和雷电僵尸骨！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    if not (hasMonkBone and hasThunkBone) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>找到了僧侣僵尸骨，那就快去找雷电僵尸骨吧！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯……看来需要的东西都带来了啊！</par>
                            <par>贫道也已经做好了其他的准备就等着施主来呢！</par>
                            <par>现在我要集中精力制造护身符，请施主稍候片刻！</par>
                            <par><event id="npc_take_charm">知道了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_charm = function(uid, args)
                    if not (server.player.hasItem(uid, '僧侣僵尸骨', 1) and server.player.hasItem(uid, '雷电僵尸骨', 1)) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呼……已经做完了！有了这个就可以破解署箭设下的不死牌困魔咒了。</par>
                            <par>一定要让这个我们道门之耻——署箭最后死在不是手下，而是<t color="red">%s</t>您的手里啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)

                    server.player.removeItem(uid, '僧侣僵尸骨', 1)
                    server.player.removeItem(uid, '雷电僵尸骨', 1)
                    server.player.addItem(uid, '毁灭护身符', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_final_fight'})
                end,
            }
        ]])
    end,

    -- the charm opens the lair, and this time he has nothing left to hide behind
    quest_final_fight = function(uid, args)
        setQuestDesp{uid=uid, '带着毁灭护身符去废矿矿山地下2层，了结署箭。'}

        setupNPCQuestBehavior('道馆_1', '书堂玄震_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和书堂玄震说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那个家伙是我们道门的羞耻啊！相信施主您会替我们处理他的！</par>
                            <par>一定要让这个我们道门之耻——署箭最后死在不是手下，而是<t color="red">%s</t>您的手里啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)
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
                [SYS_LABEL] = '关于署箭的末路',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我想道馆会有什么对付那家伙的办法的，毕竟这是关系到他们名声的事儿啊！</par>
                            <par>一定要让署箭那个堕落道士死在不是部下，而是<t color="red">%s</t>您的手中啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(lairMap, '署箭_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '了结署箭',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '毁灭护身符', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>真是像蟑螂一样生命力顽强的家伙啊！</par>
                                <par>不过你最好要明白，现在凭你的力量是不能把我怎么样的！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦？看来你还是蛮有余力啊！</par>
                            <par>你这不要脸的家伙！又来找我了啊！</par>
                            <par>和像你这样磨磨唧唧的家伙没什么可讲的！你就和它比试一下吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_kill_taoist'})
                end,
            }
        ]])
    end,

    -- his last creations, and the 不死牌 comes off the corpse
    quest_kill_taoist = function(uid, args)
        setQuestDesp{uid=uid, '署箭放出了他最后的怪物，打倒它们夺回不死牌！'}

        npcbattle.turnHostile
        {
            map     = lairMap,
            npc     = '署箭_1',
            uid     = uid,
            monster = {'尸王', '雷电僵尸', '僧侣僵尸'},
            x       = lairX,
            y       = lairY,
            say     = '署箭把毕生所学尽数放出，再也不肯开口了！',
        }
    end,

    -- carry it back to the 衙门 one last time
    quest_return_token = function(uid, args)
        setQuestDesp{uid=uid, '夺回了不死牌，回比奇省内城交给比奇城城主。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '呈交夺回的不死牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那个堕落道士署箭的末路终会是如此啊！</par>
                            <par>这只不过是他罪有应得罢了！</par>
                            <par>尽管受害匪浅，不过这事儿到此而告终了，真是不幸中的万幸啊！</par>
                            <par>万一那家伙真的带着不死怪物大军来进攻比奇省的话……那将来可就不堪设想了！</par>
                            <par><event id="npc_ask_token">那么您打算怎么处理那个不死牌呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_token = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>尽管它是很有价值的东西，但结果却造成了这样的灾难，绝对不能把它留在世上了！</par>
                            <par>一定要想个办法尽早把它毁掉。这次不管云发先生怎么劝阻，都要按照我意思来办！</par>
                            <par><event id="npc_hand_token">那我就问到这儿吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_token = function(uid, args)
                    if not server.player.hasItem(uid, '不死牌', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>不用担心不死牌也行啊！被我放在城内安全的保管着呢！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>危难之中最值得信赖和依托的人只有您啊！</par>
                            <par>不管怎么样今后还要多多关照啊！</par>
                            <par>还有这点小小的礼物作为我的一点谢意……哦，您去把包腾出点地方再来拿吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '不死牌', 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 20000)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

-- 署箭 never drops anything himself, everything comes off what he raises
mondrop.setDropOnKill
{
    {
        monster  = '僧侣僵尸',
        state    = 'quest_fight_zombie',
        setState = 'quest_taoist_fled',
        say      = '（看来需要再谈谈...）',
    },

    {
        monster  = '尸王',
        state    = 'quest_fight_king',
        give     = {SYS_GOLDNAME, 9000},
        setState = 'quest_king_dead',
        say      = '（是僵尸王...）',
    },

    -- the two zombies that stay dead are the ones carrying usable bones
    {
        monster = '僧侣僵尸',
        state   = 'quest_collect_bones',
        kills   = 3,
        once    = true,
        give    = '僧侣僵尸骨',
        say     = '（这是僧侣僵尸的骨头吗？）',
    },

    {
        monster = '雷电僵尸',
        state   = 'quest_collect_bones',
        kills   = 3,
        once    = true,
        give    = '雷电僵尸骨',
        say     = '（这是雷电僵尸骨吗？）',
    },

    {
        monster  = '尸王',
        state    = 'quest_kill_taoist',
        need     = '毁灭护身符',
        take     = '毁灭护身符',
        give     = {{'不死牌', 1}, {SYS_GOLDNAME, 12000}},
        setState = 'quest_return_token',
        say      = '（可怜的人...那种怪物就是你所说得不老不死？）',
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
                        <par>这不是还没有完成被盗灵魂任务的你所应该关心的事儿！</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>不用担心不死牌也行啊！被我放在城内安全的保管着呢！</par>
                        <par>你的等级还没有超过<t color="red">%d级</t>，这种事儿还轮不到你操心。</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>怎么现在才来啊？找了你好久了！没时间了，就只和你说一下要点吧！此前你粉碎了半兽人的阴谋后带来的古代护身符不死牌被盗了！</par>
                    <par><event id="npc_shocked">.....!</event></par>
                </layout>
            ]=])
        end,

        npc_shocked = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>一想到那不死牌是你从半兽人手中好不容易才得来的……我就感到真是无颜以对啊！可是毕竟已经到了这个地步！讨来吃的东西真是烂了人最后的肠胃啊！</par>
                    <par>因为云发先生主张那是很有学术价值的东西，不能轻易毁掉，所以就没有被处理掉而被保管在衙门之内，看来这真是个失误啊！不过既然这件事情已经发生，能够解决这件事儿的就只有您了。所以才这么急着见你。</par>
                    <par>要是这东西被乱用的话，比奇的土地上不知道又将出现什么灾难啊！</par>
                    <par>所以一定要把那个偷了不死牌的犯人绳之以法，找回不死牌。</par>
                    <par><event id="npc_saw_thief">我见过那个犯人，其实……</event></par>
                </layout>
            ]=])
        end,

        npc_saw_thief = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>.......</par>
                    <par>本官虽然听说过这个有关妖怪摄人魂魄的传闻，却真没想到确实有这样受害的人！既然这个犯人如此作恶多端，就赶快去把他捉拿归案，以免让无辜的百姓们受到更多的伤害啊！</par>
                    <par><event id="npc_ask_clue">知道了。有缉拿这个犯人的线索吗？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_clue = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>既然是跟道士有关的事儿，道馆里应该很清楚吧？而且道士们中能做出如此没有道义之事儿的人应该不多，所以去道馆打听一下的话或许就能掌握那个犯人的下落呢！</par>
                    <par><event id="npc_accept_quest">好，那我先去道馆看看！</event></par>
                </layout>
            ]=])
        end,

        npc_accept_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>去道馆见一下执客院住持<t color="red">书堂玄震</t>道士吧！</par>
                    <par>他会看在本官的颜面上最大限度的协助你的。</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
