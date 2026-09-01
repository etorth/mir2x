-- converted from Envir/QuestDiary/NQ_BASE/younghon.txt
--
-- 王小二's daughter 丽灵 came home without her soul, the trail leads to a 道士 squatting in
-- 王陵 and to the 葫芦瓶 he left behind, which turns out to be the 灵魂护卫 holding her soul
--
-- 丽灵 wakes up with the one piece of news that starts 堕落道士任务: the 道士 is after 不死牌
--
-- the neighbours 义贤 and 阿旭 fill in the rumour, 阿旭 is the one who actually saw the 道士
-- and his 葫芦瓶, and recognises it again once you are carrying it

_G.minQuestLevel = 16
_G.prequestName  = '半兽人任务'

setQuestFSMTable(
{
    -- 王大人 has sent you to his relative 王小二
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '王大人拜托你去帮他的远房亲戚王小二，去比奇省西北城门外357:273找他吧。'}

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于王小二的女儿',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去西北城门外<t color="red">357:273</t>找叫王小二的人吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王小二_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于丽灵的怪病',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦哦！<t color="red">%s</t>您来了啊！久仰久仰啊！大概您也是已经听说了这件事儿才来的吧？</par>
                            <par><event id="npc_ask_lady">就是旁边的这位小姐吧？</event></par>
                        </layout>
                    ]=], server.player.getName(uid))
                end,

                npc_ask_lady = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉……这孩子可是我家唯一的独生女儿啊！在一次赶着牛群去远方的市场的途中，我妻子生下了这孩子后就离开了这个世上。以后就是只剩下我看着我们的丽灵，辛辛苦苦把她拉扯大，可是谁知道竟会发生了这种没有天理的事儿呢……</par>
                            <par><event id="npc_ask_detail">到底发生了什么事儿，能给我详细的说一下吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_detail = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我们丽灵的爱好是在附近骑马散步，那天丽灵也是像往常一样进了马厩骑上专门为她准备的浑身雪白的白马出去散步了。可是已经过了回来的时间，太阳都落山了丽灵却一直没有回来！正当我们无法再等，集合了村里的壮丁高举火把要去寻找的时候，丽灵却突然回来了，可是……就像你现在看到的这样，回来的只是个没有知觉的肉身了！</par>
                            <par><event id="npc_ask_doctor">诊断过丽灵小姐为何变成这样的原因了吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_doctor = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>四处寻医求治，无论是什么医生大夫还是巫师、巫婆全都找过了，可还是一点儿用都没有。结果就只好根据剩下的一种传闻作为线索来请<t color="red">%s</t>您来帮忙了。</par>
                            <par><event id="npc_ask_rumour">能告诉我传闻的内容是什么吗？</event></par>
                        </layout>
                    ]=], server.player.getName(uid))
                end,

                npc_ask_rumour = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>不久之前开始流传一个关于专门摄取夜间外出的人魂魄的妖怪的传闻。虽然不敢确定，但是据说也有和我们的丽灵一样遇害的人。所以拜托您一定要调查出这传闻的真相，让我们的丽灵恢复到原来的样子啊！</par>
                            <par>马回来的方向是西北方，是<t color="red">半兽洞穴</t>的方向，那附近最值得怀疑。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_heard_story'})
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return
            {
                [SYS_HIDE] = true,
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>王小二家在比奇省西北城门外357:273，先去听听他女儿到底出了什么事。</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    -- the horse came back from the 半兽洞穴, 黄晶 is the scholar working down there
    quest_heard_story = function(uid, args)
        setQuestDesp{uid=uid, '丽灵的白马是从半兽洞穴方向回来的，去半兽洞穴1层找学者黄晶打听吧。'}

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于王小二的女儿',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>找到他家了吧！那就多多拜托了！最近由于那个妖怪的传闻搅得民心惶惶的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王小二_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于丽灵的怪病',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果我们丽灵没有发生这种事儿的话，我也不会对这种妖怪摄人魂魄的事儿十分在意……可是既然发生了这种事儿也觉得的确是有些诡异啦！先去找找在这附近住的人听听他们是怎么说的吧！没准儿能得到对调查这件事儿有帮助的情报呢！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽洞穴1层_D001', '黄晶_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听半兽洞穴的怪事',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>听说半兽洞穴连接着天然洞穴和半兽天然洞穴，并与比奇县矿山由连接通路相连着。唔……我现在有点忙，如果没有什么特别事儿的话就不要来打扰我！</par>
                            <par><event id="npc_ask_strange">不知道最近半兽洞穴里有没有什么异常的事情发生？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_strange = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我叫黄晶，不久前接替了叫做云发的学者继续他的研究。可是去半兽洞穴2层的<t color="red">王陵</t>调查的时候被一个不知哪儿来的道士赶了出来！我还是平生头一次遇到这种丝毫不讲道义并且不与官府的工作合作的道士呢！而且说话还十分不客气……真是……唉，算了。</par>
                            <par><event id="npc_offer_help">我去跟他说说看吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_offer_help = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你会帮我去说？由于半兽人的骚动，占用了很多时间，所以要尽快再展开调查。可是由于那个家伙的妨碍真是把我郁闷死了！要是你能把那个混蛋道士弄出来的话，我会跟比奇省联系一下让您得到辛苦费的！</par>
                            <par>那个道士呆的王陵在<t color="red">半兽洞穴2层225:175</t>。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_help_scholar'})
                end,
            }
        ]])

        -- 义贤 sells 王小二 his horses and knows which way 丽灵 came home from
        setupNPCQuestBehavior('比奇县_0', '义贤_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听丽灵失踪那天的事',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>王小二那个朋友的女儿？可怜的小八啊……太年轻了只知道工作，还死了老婆，唯一剩下的一个女儿也变成了那样，以后可怎么办呢！</par>
                            <par><event id="npc_ask_that_day">能给我讲讲她女儿失踪的那天发生的事儿吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_that_day = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>丽灵唯一的爱好就是骑着马去散心！本来小八他对女儿太过疼爱所以一直把她关在家里，所以我劝他别把女儿憋出病来。于是小八就花巨额给女儿买了一匹白马让她骑。丽灵骑上那匹一点杂色都没有的白马绕着村子那么一转，所有年轻男子的视线就都被吸引过去了！真的是个非常可爱的女孩啊！唉……</par>
                            <par>咳！现在不是说这种话的时候……丽灵那天晚上很晚才伏在马背上回到马厩……不……确切地说是马儿驮着那个已经没有了魂魄的孩子自己找回家的。马回来的方向是西北方，是<t color="red">半兽洞穴</t>的方向。从半兽洞穴附近的杂草被马蹄踩过的情形看来就是在那附近遭遇了不幸！</par>
                            <par><event id="npc_ask_suspect">您觉得哪儿最可疑呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_suspect = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我已经跟小八说过了半兽洞穴是最值得怀疑的地方这样的话。这也是他之所以特地把常在半兽洞穴附近活跃的您叫来，求你办这件事儿的原因啊！</par>
                            <par>好好去调查一下半兽洞穴的话，一定能够找到什么线索的。丽灵就像我的亲生女儿一样，你可一定要把这件事儿调查个水落石出啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        -- 阿旭 is the only one who has seen the 道士 and lived
        setupNPCQuestBehavior('比奇县_0', '阿旭_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '打听摄魂妖怪的传闻',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯？你说什么？啊，你说王小姐啊？王小二正伤心着呢。还不是因为她女儿……</par>
                            <par><event id="npc_ask_monster">您听说过关于那个妖怪摄人魂魄的传闻吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_monster = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哪里只是传闻而已啊？这可是我亲眼所见的！</par>
                            <par><event id="npc_ask_saw">您能给我详细地讲一下吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_saw = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那是大概在一个月之前的事儿了！凌晨的时候我要去外面小解，偶然看见什么东西从墙角闪过。正好那天没出月亮黑漆漆的我没能看清楚，但好像是个男子的模样。我还以为是个贼就没出声儿，原地不动的站在那儿看，只见他解开腰间的一个好像是<t color="red">葫芦瓶</t>模样的东西拿在手里轻轻摇晃，嘴里还念念有词的不知在嘟囔着什么。看了一会儿我觉得他行为异常刚想出声赶走他，那个家伙向我呆的地方～嚯…得一下，被他眼光盯到的那一瞬间我一下子喘不过气来，出了一身冷汗，可是就在我想“这下子可完蛋了”的那一瞬间，那个妖怪就不知道消失到哪儿去了！</par>
                            <par><event id="npc_ask_after">那后来怎么样了呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_after = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉！甭提了！那家伙消失之后我身上就渐渐起了好多绿色的斑点，而且开始发高烧、吐血……好不容易回到卧室吃了从爷爷那里传下来的秘药才算捡回了这条性命。真的……我还以为死定了呢！就那么看了我一眼我就好像中了剧毒，看来那妖怪真是神通广大啊！</par>
                            <par><event id="npc_guess_taoist">这好像是道士的招数啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guess_taoist = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>道士？世上哪有对无辜的人用毒术的道士啊？这一定是害人的妖怪没错儿！唉……不会有错儿的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return
            {
                [SYS_HIDE] = true,
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>半兽洞穴1层有个叫黄晶的学者常年在那儿做研究，那边的怪事她最清楚。</par>
                            <par>比奇省里的马商义贤和邻居阿旭也都有话说，顺路听听吧。</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    -- talk the 道士 out of 王陵, he only leaves in exchange for your story
    quest_help_scholar = function(uid, args)
        setQuestDesp{uid=uid, '答应黄晶去劝走王陵里的道士，王陵在半兽洞穴2层225:175。'}

        setupNPCQuestBehavior('比奇县_0', '阿旭_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和阿旭说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>连王小姐都被妖怪摄走了魂魄，真是没有天理啊……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('王陵_DM002', '无理的道士_1', uid,
        [[
            return getUID(), getQuestName(), server.player.getName
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '劝道士离开王陵',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>对不起，我现在正在进行重要的研究，请别来妨碍我，最好走开！</par>
                            <par><event id="npc_ask_purpose">你在这儿要做什么呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_purpose = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>没必要告诉你吧！</par>
                            <par><event id="npc_ask_scholar">为什么把叫做黄晶的学者赶走呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_scholar = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>...</par>
                            <par>......</par>
                            <par>好像在哪儿见过你……你不就是那个打败了骷髅精灵和半兽勇士后得到了什么古代护身符的人吗？</par>
                            <par><event id="npc_admit">嗯，是我，不过……</event></par>
                        </layout>
                    ]=])
                end,

                npc_admit = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哈哈哈……这真是太好了！那么我有一个提议，你看如何？我是个对你的经历非常感兴趣的人，如果你能够详细的把你是如何破解半兽人诡计的过程讲给我听的话，我就会离开这儿让那个学者进行研究，怎么样？</par>
                            <par><event id="npc_tell_story">好吧！我都告诉你！</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_story = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>....</par>
                            <par>.....</par>
                            <par>噢噢！这么说不死牌已经完整了啊！那个时候我好像在半兽洞穴1层的什么地方见过你。那么不死牌怎么处理了呢？</par>
                            <par><event id="npc_tell_token">被放在比奇省衙门保管起来了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_token = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>原来如此……可要好好看管才是啊！别让居心不良的人给偷走……</par>
                            <par>哈哈哈</par>
                            <par>那么我会按照约定离开这里，告诉那个学者来这儿好好调查吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_taoist_left'})
                end,
            }
        ]])
    end,

    -- tell 黄晶 the way is clear
    quest_taoist_left = function(uid, args)
        setQuestDesp{uid=uid, '道士已经答应离开王陵，回半兽洞穴1层告诉黄晶吧。'}

        -- he keeps his word and stops blocking 王陵, but talk to him again and he is already
        -- plotting, this is the first hint of 堕落道士任务
        setupNPCQuestBehavior('王陵_DM002', '无理的道士_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '再看看那个道士',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>没想到在附近竟有这种宝物，我要另外进行试验……嘿嘿嘿</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('半兽洞穴1层_D001', '黄晶_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '回报黄晶',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去办的事儿怎么样了？</par>
                            <par><event id="npc_report_done">嗯，我让他离开那儿了。您可以去进行调查了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_report_done = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>幸亏了你啊！竟然有他那种道士……真是……，不管怎么样我会跟比奇省联系一下，<t color="red">比奇城城主</t>大人会给你辛苦费的！谢谢你帮助我的工作啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_scholar_thanks'})
                end,
            }
        ]])
    end,

    -- the 城主 pays up, and hands over the gourd 黄晶 found in 王陵
    quest_scholar_thanks = function(uid, args)
        setQuestDesp{uid=uid, '黄晶已经写信给比奇省，去比奇省内城找比奇城城主领辛苦费吧。'}

        setupNPCQuestBehavior('半兽洞穴1层_D001', '黄晶_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和黄晶说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>托您的福，研究再次展开了！</par>
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
                [SYS_LABEL] = '领取辛苦费',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我收到了黄晶的信，正在等你呢！好久不见，你又在那儿作了侠义之举啊！好吧！这是给你的辛苦费和东西。</par>
                            <par><event id="npc_ask_bottle">这个葫芦瓶好像不是我的东西啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_bottle = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唔？这个是黄晶先生调查王陵的途中发现的，以为是你落下的东西所以送了过来。嗯，既然不是那怎么办呢……算了，你就拿走吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '灵魂护卫', 1)
                    server.player.addItem(uid, SYS_GOLDNAME, 5000)
                    server.quest.setState(questUID, {uid=uid, state='quest_got_bottle'})
                end,
            }
        ]])
    end,

    -- the gourd is what 丽灵's soul is trapped in
    quest_got_bottle = function(uid, args)
        setQuestDesp{uid=uid, '从比奇城城主那儿得到了王陵里发现的葫芦瓶，拿去给王小二看看吧。'}

        setupNPCQuestBehavior('比奇县_0', '比奇城城主_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和比奇城城主说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我计划着手研究一下不死牌。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        -- 阿旭 saw this very gourd on the 道士 that night, showing it to him confirms the two
        -- halves of the story are the same one
        setupNPCQuestBehavior('比奇县_0', '阿旭_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '给阿旭看葫芦瓶',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂护卫', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>现在想来还挺想知道那个妖怪带着的葫芦瓶到底是用来做什么的东西……不过可不再想和那个妖怪碰面了。好奇心可是要用性命换的啊！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯？那个葫芦瓶！能给我看一下那个葫芦瓶吗？</par>
                            <par><event id="npc_show_bottle">给他看灵魂护卫。</event></par>
                        </layout>
                    ]=])
                end,

                npc_show_bottle = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯！没错，这个就是那妖怪带着的葫芦瓶。那人是摄人魂魄的妖怪，可是这个东西怎么又带在您身上呢？</par>
                            <par>不知道这个葫芦瓶里有没有医治王小姐的办法？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王小二_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '给王小二看葫芦瓶',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂护卫', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>丽灵现在还没能恢复意识呢！您还没能找到让这孩子清醒的办法吗？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>丽灵现在还没能恢复意识呢！不知道您是否找到了让这孩子清醒的办法……</par>
                            <par><event id="npc_show_bottle">给他看灵魂护卫！</event></par>
                        </layout>
                    ]=])
                end,

                npc_show_bottle = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这是干什么用的东西啊？难道用这个葫芦瓶能治好丽灵的病？</par>
                            <par><event id="npc_open_bottle">请打开瓶塞看看吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_open_bottle = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂护卫', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>咦？那个葫芦瓶呢？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦，好的……可是打开瓶塞干嘛……啊，这……</par>
                            <par>丽灵啊！你醒过来了吗？哈哈哈……丽灵啊！</par>
                            <par><t color="red">%s</t>，真是太感谢您了！丽灵已经醒过来了！我的丽灵儿啊！这是我表示我谢意的一点东西，虽然不够丰厚，但希望你不要嫌弃，把这收下吧！</par>
                            <par>啊，要是你的包里没有空余的地方，快点腾出了地儿再来！哦呵呵呵！真是太感谢啦！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)

                    server.player.removeItem(uid, '灵魂护卫', 1)
                    server.player.addItem(uid, '牛肉', 5)
                    server.player.addItem(uid, SYS_GOLDNAME, 10000)
                    server.quest.setState(questUID, {uid=uid, state='quest_liling_awake'})
                end,
            }
        ]])
    end,

    -- 丽灵 heard everything from inside the gourd, and that is what sets up 堕落道士任务
    quest_liling_awake = function(uid, args)
        setQuestDesp{uid=uid, '丽灵已经苏醒，去听听她在葫芦瓶里都听到了什么。'}

        setupNPCQuestBehavior('比奇县_0', '王小二_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和王小二说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>谢谢你这么尽力帮助我们丽灵啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '丽灵_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '听丽灵说经过',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯... 嗯...</par>
                            <par><event id="npc_wake_up">王小姐，你醒过来了吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_wake_up = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这声音……咦……你是谁啊？</par>
                            <par>哦！不不，我好像认识你啊！我听到了你在洞穴中的话。打败半兽勇士得到不死牌的不就是你吗？</par>
                            <par><event id="npc_how_know">你是怎么知道的？</event></par>
                        </layout>
                    ]=])
                end,

                npc_how_know = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我都听见了，我在葫芦瓶里全都……那个坏道士和你的话我全都听见了！</par>
                            <par><event id="npc_in_bottle">你说是在葫芦瓶里？</event></par>
                        </layout>
                    ]=])
                end,

                npc_in_bottle = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我骑着马在散步的时候那个道士出现了。我一眼就看出那个道士没安什么好心眼儿，就骑着马慌忙逃跑，可是还是在半兽洞穴附近被抓到了。那个家伙念了些什么奇怪的咒语我就被装进了葫芦瓶。不知多久后我才知道被关进了葫芦瓶里！</par>
                            <par><event id="npc_soul_in_bottle">原来你的灵魂被装进了葫芦瓶里啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_soul_in_bottle = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>对！他看起来好像是要拿我的灵魂做某种实验。我在葫芦瓶里听到的话很多，可是我的见识太少一句都听不明白！不过现在倒是明白了一点儿，那家伙好像为了去做什么事儿把这个葫芦瓶扔下离开了。因为要得到更贵重的东西，所以不需要这个了！</par>
                            <par><event id="npc_what_thing">那是什么呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_what_thing = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par><t color="red">不死牌</t>！那个混蛋道士所觊觎的东西就是不死牌。通过和您的对话得知了不死牌下落的那个道士，现在正要潜入衙门去盗取不死牌呢！如果让他得逞的话又不知会发生什么样的灾难呢，所以赶快去衙门看看吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '王大人_1'), getUID(), getQuestName(), minQuestLevel, prequestName,
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
                        <par>唉呀！就像我的亲生孩子一样的啊！怎么会偏偏遇上这种事儿呢……</par>
                        <par>不过这种事儿，还是得找个像破了<t color="red">半兽人</t>阴谋那样的人来办才行啊。</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>唉呀！就像我的亲生孩子一样的啊！怎么会偏偏遇上这种事儿呢……</par>
                        <par>不过你的等级还没有超过<t color="red">%d级</t>，这种邪门的事儿可不能让你去冒险啊！</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>啊！正好想要再派人去找你来呢，其实，是因为又有点奇怪的事儿发生，所以需要你的帮助！</par>
                    <par><event id="npc_ask_what">是什么事儿呢？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_what = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>事情是这样的。我有个经营牛生意的远房亲戚叫做<t color="red">王小二</t>，可是最近不知怎么回事儿，他的女儿好像中了邪似的灵魂脱壳了……为了能够让那个孩子恢复知觉用尽了各种各样办法，却一点效果都没有！而且最近又流传着很多诡异的传闻……</par>
                    <par>王小二听说这次您粉碎了半兽人的阴谋的事迹，所以就来求我拜托你了……他说你算是他最后的希望了……</par>
                    <par><event id="npc_ask_rumour">有什么诡异的传闻呢？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_rumour = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>唔嗯……这个你直接去听听就知道了！不过这件事儿能不能就拜托给你呢？他们会重重谢你的！</par>
                    <par><event id="npc_accept_quest">好吧！去哪儿找他呢？</event></par>
                </layout>
            ]=])
        end,

        npc_accept_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你去比奇省西北城门之外<t color="red">357:273</t>就能找到他们家了。王小二这个名字很好记吧……</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
