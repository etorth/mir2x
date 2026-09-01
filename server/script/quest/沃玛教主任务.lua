-- converted from Envir/QuestDiary/NQ_EXTRA/umyun.txt
--
-- the last surviving 沃玛 cultist wants the thing he helped summon put back down. 沃玛教主 was
-- raised on a 灵魂明珠 packed with the souls of sacrifices — one of them the old man's own
-- brother — and breaking it shuts the gate and frees them
--
-- the 沃玛金牌 you need to reach the priests' hall has been sold on twice, so the first half of
-- the quest is chasing it back: 王大人 -> 无名商人, who will only trade it for the 地狱神钟
-- hidden behind the 石堆 mechanism in 沃玛神殿1层
--
-- 牛老道 poses as a 道馆 priest to take the 灵魂明珠 off you, then drops the act. legacy
-- despawned him and spawned the monster; mir2x NPCs cannot despawn yet, so
-- npcbattle.turnHostile silences him instead, see quest/include/npcbattle.lua

_G.minQuestLevel = 20

_G.hermitMap  = '无名老人隐居地_1_009'
_G.priestMap  = '沃玛神殿师徒关_D023_001'
_G.priestX    = 17
_G.priestY    = 21

local mondrop   = require('quest.include.mondrop')
local npcbattle = require('quest.include.npcbattle')

setQuestFSMTable(
{
    -- 王铁匠 has told you these relics are worth money, go and turn one up
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '沃玛神殿的古董能卖上价钱，去猎杀沃玛战士找一件出来。'}

        setupNPCQuestBehavior('道馆_1', '王铁匠_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于沃玛神殿的古董',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>据说王大人喜欢收集这些古董，去比奇省看看怎么样？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- 王大人 buys it, and lets slip that a hermit near 道馆 has a whole hoard of them
    quest_sell_medal = function(uid, args)
        setQuestDesp{uid=uid, '找到了沃玛金牌，拿去给道馆的王铁匠看看。'}

        setupNPCQuestBehavior('道馆_1', '王铁匠_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '鉴定沃玛金牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是年代十分久远的东西啊！不过不知道还有什么用处，好像除了作为古董没什么别的价值了……</par>
                            <par>听说比奇省的富豪<t color="red">王大人</t>收集这些东西，拿去卖给他换成钱要比就这么带着更好！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '把沃玛金牌卖给王大人',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '沃玛金牌', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>好像没有带着沃玛金牌啊！这是怎么回事儿？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您又为何事而来呢？</par>
                            <par><event id="npc_show_medal">给你看沃玛金牌来了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_show_medal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！真是很久以前的古董了啊!能告诉我这是什么东西吗？</par>
                            <par><event id="npc_dont_know">在沃玛神殿中偶然得到的，但我也不知道是什么东西。</event></par>
                        </layout>
                    ]=])
                end,

                npc_dont_know = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唔……那么对你来说没什么用啊！这样吧！就把这个沃玛金牌卖给我，我会给你个好价钱的……</par>
                            <par>怎么样？能卖给我吗？</par>
                            <par><event id="npc_sell">好的，卖给你！</event></par>
                            <par><event id="npc_keep">现在还不想卖！</event></par>
                        </layout>
                    ]=])
                end,

                npc_keep = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵……可惜啊！好久没有看到这样的古董了，特别想买下来……</par>
                            <par>如果你改变主意了的话，什么时候来都行！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_sell = function(uid, args)
                    if not server.player.hasItem(uid, '沃玛金牌', 1) then
                        return
                    end

                    -- he pays more to someone who has already worked for 比奇商会
                    if server.player.getQuestState(uid, '比奇商会') == SYS_DONE then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>您为了我们比奇商会做了很多事情，一定会给你个高价来买的，这样吧！收下这15万钱，不……20万钱！</par>
                                <par><event id="npc_thanks">谢谢！</event></par>
                            </layout>
                        ]=])
                        server.player.addItem(uid, SYS_GOLDNAME, 200000)

                    elseif server.player.getQuestState(uid, '比奇商会') ~= nil then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>您为了我们比奇商会做了很多事情，一定会给你个高价来买的，这样吧！收下这10万钱！</par>
                                <par><event id="npc_thanks">谢谢！</event></par>
                            </layout>
                        ]=])
                        server.player.addItem(uid, SYS_GOLDNAME, 100000)

                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>那么给你五万钱！</par>
                                <par><event id="npc_thanks">谢谢！</event></par>
                            </layout>
                        ]=])
                        server.player.addItem(uid, SYS_GOLDNAME, 50000)
                    end

                    server.player.removeItem(uid, '沃玛金牌', 1)
                end,

                npc_thanks = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵！越看越神奇啊!对了，你听说过关于那个有很多沃玛神殿中物品的老人的故事吗？</par>
                            <par><event id="npc_never_heard">没有，没听过！</event></par>
                        </layout>
                    ]=])
                end,

                npc_never_heard = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>有传闻说在道馆附近的什么地方有一位独自生活的老人，他有很多过去沃玛教中曾经使用过的古董。我也听说了这事儿并派人去他那儿买，可是都失败了！如果你去沃玛神殿附近打猎的时候遇到那个老人的话，就告诉他我王某人想要出高价购买他的东西！那个无名老人隐居在<t color="red">道馆西北部的小山谷里</t>的一个小茅屋中，不过<t color="red">进入他隐居地的入口</t>可不太好找！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_hermit'})
                end,
            }
        ]])
    end,

    -- he will not talk until you bring back the journal he lost in the temple
    quest_find_hermit = function(uid, args)
        setQuestDesp{uid=uid, '无名老人对沃玛神殿讳莫如深，去沃玛神殿猎杀沃玛卫士，找到他丢失的无名日志。'}

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于那位无名老人',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>去找住在道馆附近的老人转告他我王某人要出高价购买跟沃玛教有关的古董。那个无名老人隐居在<t color="red">道馆西北部的小山谷里</t>的一个小茅屋中，不过<t color="red">进入他隐居地的入口</t>可不太好找！</par>
                            <par>不知道那个老人是有何种内力的人啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '无名日志', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>还是别进沃玛神殿啦！为了贪图宝物可是会丢了性命的！我可是警告你了啊！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这，这不是我不久前丢失的日志嘛！</par>
                            <par><event id="npc_ask_temple">不能告诉我跟沃玛神殿有关的事情吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_temple = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>.......</par>
                            <par>唉！……既然你带来了这本日志，那么再隐瞒也没有用了！</par>
                            <par>不过……这件事儿实在太危险了！呃咳，呃咳！插手管这件事儿可会危及你的性命啊！所以你还是不要好奇，回去吧！</par>
                            <par><event id="npc_insist">但我一定要知道。</event></par>
                            <par><event id="npc_threaten">这么慌张的样子，看来是有什么不可告人的勾当吧！我要去衙门告发你！</event></par>
                        </layout>
                    ]=])
                end,

                npc_threaten = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你敢试试看？我一个人与那些狰狞的恶魔战斗了数十年之久，难道还会把你这个小毛孩子的威胁放在眼里？</par>
                            <par>跟你这种不讲道理的人什么都不想说了……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_insist = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呃咳，呃咳，看你一身正气的样子，跟你说说也无妨。我已经老了，要做的事儿也到最后了……唉！好吧！我会告诉你一切的。</par>
                            <par>那么你想从哪儿听起呢？</par>
                            <par><event id="npc_ask_history">我想听听关于沃玛神殿的事情。</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_history = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很久以前……我们的祖先因迷失了返回故乡的路而迷惑彷徨时，一部分开拓者发现了这个地下寺庙，他们探查这个寺庙并且得到了一本古代文书。这文书上写着把某种超越性的存在召唤出地面的方法。但因为那个方法十分残忍，要献上活的祭物才行，所以无论如何都不能采用。</par>
                            <par>但是没能返回故乡遭遇挫折的人们最后还是选择了这条路，秘密的成立了邪教，开始四处抓人作为活的祭物献上。官府当然会派出官兵来捉拿邪教徒，所以剩下的邪教徒便藏到了这沃玛神殿里来。于是在这漆黑一片的地下，官兵和教徒们展开了激烈的血战……</par>
                            <par><event id="npc_ask_after">那么后来怎么样了呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_after = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>邪教徒们坚信如果召唤出这超越性的存在就能得到援助，所以不顾数量劣势地进行殊死抵抗，熟悉了内部构造和适应了黑暗的教徒们与官兵大战了三天三夜，重创了官兵们。但是那时却发生了未曾预料到的事情，寺庙中充斥的血腥味儿让“那个”睁开了眼睛。</par>
                            <par><event id="npc_ask_king">难道“那个”就是沃玛教主？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_king = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>可不是嘛！睁开眼睛的沃玛教主打开了地狱之门召唤出自己的部下，开始攻击寺庙内的人们。邪教徒们开始还以为是自己祈愿的神的使者降临了，所以都欣喜万分，可是没想到那些怪物们却两者不分，攻击所有眼中看到的人类！就这样……邪教徒们坚信的所谓超越者其实不过是就连你都知道的魔鬼头目！互相残杀的人们最后终于无法抵挡那些怪物，全都死掉了！</par>
                            <par><event id="npc_ask_survivor">可是假如没有生还者的话，那么您怎么会如此详细的知道当时发生的情况呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_survivor = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵，我怎么能这么详细的知道当时发生的事儿呢，就像你察觉的那样，我就是当时那个杀戮现场唯一幸存下来的人啊！我也是邪教徒！召唤沃玛教主到世上来的……但是从那天之后，我虽然活着，但却生不如死啊！这都是因为熟悉这沃玛神殿的构造，并知道除掉沃玛教主办法的人除了是邪教徒的我以外，就没有别人了，所以我才一直苟活到了今天……</par>
                            <par>但是现在我已经年劳力衰，即使知道办法却也不能去做了！所以只好不顾廉耻的要拜托您去除掉沃玛教主。不是为了我这个老家伙，而是为了能够帮那些屈死的冤魂们报仇啊！你能帮我这一把吗？</par>
                            <par><event id="npc_accept_help">我会尽全力帮您的！</event></par>
                            <par><event id="npc_refuse_help">我可不想去做那么危险的事情！</event></par>
                        </layout>
                    ]=])
                end,

                npc_refuse_help = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这是没有办法的事儿啊！我也没有资格去埋怨你，就让我这条命直到战死为止吧，就算是死也一定要除掉那个魔鬼……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_accept_help = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>有了你的帮助，我就看到了希望啊！那么我来告诉你这办法吧！</par>
                            <par>沃玛教主原本是通过人身供养而制造出的<t color="red">灵魂明珠</t>才复活的。也就是说，灵魂明珠是通过残忍的牺牲祭礼封入死去的冤魂而制造出来的。但当时邪教徒们由于中途受到官兵攻击而没能彻底完成灵魂明珠。因此沃玛教主自身的力量并没能完全的发挥出来，只是用它打开了阴间之门唤出了部下们。所以只要能够夺回灵魂明珠并把它毁掉的话，就可以锁上阴间之门削弱沃玛教主的力量。那么被锁住的冤魂们就都能够升天了。</par>
                            <par>不过，首先要找到<t color="red">沃玛金牌</t>才行。有那个才能出入秘密地区……进不了秘密地区的话就算找到灵魂明珠也无法得到破坏它的办法。所以你能先去找找沃玛金牌吗？既然找回了我记录通往秘密地区入口的日志，有了沃玛金牌的话就可以解决这件事的关键了……</par>
                            <par><event id="npc_sold_it">可是沃玛金牌已经被我卖给王大人了……</event></par>
                        </layout>
                    ]=])
                end,

                npc_sold_it = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你说什么？你把那个给卖了？也就是说你找到了那个金牌？我过去数十年为了找它而费尽了心血都没……哦，现在就没问题了，那么快去<t color="red">王大人</t>那儿把那个金牌找回来吧！那可是非常重要的东西啊！你只要能把它找来的话就会有除掉沃玛教主的办法了！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '无名日志', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_ask_wang'})
                end,
            }
        ]])
    end,

    -- 王大人 bought it, then passed it to a trader heading for the desert
    quest_ask_wang = function(uid, args)
        setQuestDesp{uid=uid, '沃玛金牌卖给了比奇省的王大人，去把它要回来。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快去王大人那儿要回那个金牌吧！那可是非常重要的东西，是除掉沃玛教主必需的东西！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '要回沃玛金牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好像你找到住在道馆附近的老人了吧？</par>
                            <par><event id="npc_want_back">其实……我是来要回沃玛金牌的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_want_back = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯？又发生什么事儿了？要拿走沃玛金牌？</par>
                            <par><event id="npc_tell_story">请您听一下我在道馆遇到的无名老人所说的话吧！…… ……</event></par>
                        </layout>
                    ]=])
                end,

                npc_tell_story = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯……原来还有这等事情啊……我不知竟是如此，结果又犯了一个错误啊……其实那个沃玛金牌已经不在我的手中了！要去沙漠的贸易商需要这种物品作为礼物送给当地土著部落族长，所以来求我。本来我不想给他的，但是那是个非常重要的贸易线，实在没办法拒绝啊！不过既然除掉沃玛教主必需那个金牌的话……尽快去追那个人还来得及，那个贸易商说要去沙漠，快去追的话还能追上。</par>
                            <par>您的侠义心肠实在是了不起啊！祝你一定能够找到那个去沙漠的贸易商！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_trader'})
                end,
            }
        ]])
    end,

    -- the trader will only swap it for something worth more
    quest_find_trader = function(uid, args)
        setQuestDesp{uid=uid, '沃玛金牌转手给了要去沙漠的贸易商，去边境城市找无名商人。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊？你说那个已经被卖给去沙漠的贸易商了……竟有这样浪费的事儿！</par>
                            <par>快去找那个贸易商吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '无名商人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '要回沃玛金牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>来找我有什么事儿吗？</par>
                            <par><event id="npc_want_medal">我是为要回沃玛金牌而来的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_want_medal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>沃玛金牌是我从比奇省的王大人那儿购买的古董……啊，看来你就是把沃玛金牌卖给王大人的那个叫<t color="red">%s</t>的人吧！可是现在你又为什么需要它呢？</par>
                            <par><event id="npc_explain">其实是为了把沃玛教主……</event></par>
                        </layout>
                    ]=], server.player.getName(uid))
                end,

                npc_explain = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>....</par>
                            <par>嗯……原来是这样啊！虽然我能够理解，不过对我来说，这东西是为开辟贸易道路而精心准备的礼物，所以要我让步可不容易，你还是回去吧！</par>
                            <par><event id="npc_ask_price">那你要怎么样才能把这个沃玛金牌还给我呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_price = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>如果你一定要这个沃玛金牌的话，可以拿别的东西来和我交换。如果你能给我找来沃玛神殿中比沃玛金牌更有价值的古董的话我就会把这个东西还给你的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_bell'})
                end,
            }
        ]])
    end,

    -- the old man knows where a better relic is walled up
    quest_find_bell = function(uid, args)
        setQuestDesp{uid=uid, '贸易商要一件更值钱的沃玛神殿古董才肯换，去问无名老人。'}

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '关于沃玛金牌',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯！你说贸易商要求用其他能代替沃玛金牌的东西吗？咳！那个人也必须给当地土著送这种东西做礼物，实在是没办法啊！</par>
                            <par>啊！或许那个十分了解沃玛神殿的老人知道他要什么呢！去那个无名老人那儿打听一下吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '无名商人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名商人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没找来交换沃玛金牌的东西吗？离我要出发去沙漠的日子已经没有几天了，你还是尽快吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问可交换的古董',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯……他说要想拿回沃玛金牌需要有合适的东西交换才行？但是我现在有的物品中没有合适的啊……</par>
                            <par><event id="npc_offer_search">那么我去沃玛神殿找找吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_offer_search = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>除此之外好像就没有别的办法了！不过正好我知道准确的地方，那里面的宝物虽然是比沃玛金牌还要贵重，但因我能力有限无法得到啊!具体的位置在<t color="red">沃玛神殿1层的207:47</t>附近。可能仔细观察周围的石堆就会发现机关装置。不过要准确的操作那些机关才行。要是错误操作了的话，即使进去了也会到达奇怪的地方，所以一定要提起精神谨慎处理才行啊！</par>
                            <par>沃玛神殿一层的那地方是沃玛教的祭司们进行牺牲祭礼的地方……有无数的人冤死在那儿，阴气惨然……所以无论如何一定要多多小心才行啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_open_vault'})
                end,
            }
        ]])
    end,

    -- work the 石堆 mechanism, the 地狱神钟 is on the shelf behind it
    quest_open_vault = function(uid, args)
        setQuestDesp{uid=uid, '沃玛神殿1层207:47的石堆后面藏着机关，进去找出比沃玛金牌更贵重的宝物。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>具体的位置在沃玛神殿1层的207:47附近。可能仔细观察周围的石堆就会发现机关装置。</par>
                            <par>发现了装置一定要小心操作啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('沃玛神殿1层_D022', '石堆_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '查看石堆',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>石堆之间露出一处古旧的机关装置。</par>
                            <par><event id="npc_pull_lever">小心地扳动机关。</event></par>
                        </layout>
                    ]=])
                end,

                npc_pull_lever = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>石堆缓缓移开，露出了通往祭礼间的暗道。</par>
                            <par><event id="%s" close="1">进去看看</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.spaceMove(uid, '沃玛神殿1层_D022_001', 64, 23)
                end,
            }
        ]])

        setupNPCQuestBehavior('沃玛神殿1层_D022_001', '书架_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '查看书架',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>积满灰尘的书架上摆着一口小巧的古钟，钟身上刻满了扭曲的人脸。</par>
                            <par><event id="npc_take_bell">取走那口钟。</event></par>
                        </layout>
                    ]=])
                end,

                npc_take_bell = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是年代十分久远的东西啊！不过不知道还有什么用处，好像除了作为古董没什么别的价值了……</par>
                            <par>听说比奇省的富豪王大人收集这些东西，拿去卖给他换成钱要比就这么带着更好！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '地狱神钟', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_trade_bell'})
                end,
            }
        ]])
    end,

    -- swap the bell for the medal
    quest_trade_bell = function(uid, args)
        setQuestDesp{uid=uid, '取得了地狱神钟，拿去和边境城市的无名商人换回沃玛金牌。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '给无名老人看地狱神钟',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '地狱神钟', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>地狱神钟哪儿去了？如果没有那个的话就拿不来沃玛金牌了！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>对，就是那个东西！用那个东西的话就完全可以拿回沃玛金牌了！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '给王大人看地狱神钟',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是地狱神钟啊……真是让我贪心的东西啊！但是已经有主儿了，我也只能无可奈何了。用这个完全可以换回沃玛金牌的！</par>
                            <par>那么快去无名老人那儿吧！祝你一定能除掉沃玛教主！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('边境城市_01', '无名商人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '用地狱神钟交换',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '地狱神钟', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>地狱神钟在哪儿啊？要是打算骗我的话，还是算了吧！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>带来可以代替沃玛金牌的东西了？</par>
                            <par><event id="npc_show_bell">这个宝物是地狱神钟。</event></par>
                        </layout>
                    ]=])
                end,

                npc_show_bell = function(uid, args)
                    if not server.player.hasItem(uid, '地狱神钟', 1) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哇……这个看来确实是比沃玛金牌更有价值啊！好吧！成交！拿走沃玛金牌吧！</par>
                            <par>您拿给我的神钟将会为开拓比奇省的未来派上大用场的！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '地狱神钟', 1)
                    server.player.addItem(uid, '沃玛金牌', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_hunt_orb'})
                end,
            }
        ]])
    end,

    -- the orb is carried by one of the 沃玛护卫
    quest_hunt_orb = function(uid, args)
        setQuestDesp{uid=uid, '拿回了沃玛金牌，把它交给无名老人。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交出沃玛金牌',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '沃玛金牌', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>沃玛金牌在哪儿呢？你难道忘了那是除掉沃玛教主所必需的东西了吗？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>拿来沃玛金牌了？</par>
                            <par><event id="npc_hand_medal">是的，从贸易商那儿拿回沃玛金牌了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_medal = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>做得好！现在已经拿到了沃玛金牌，终于该轮到去夺取灵魂明珠了！因为灵魂明珠被<t color="red">沃玛护卫</t>们轮流看守着，所以不太好找！不过如果您要是能够消灭所有沃玛护卫的话，也没有找不到的道理！</par>
                            <par><event id="npc_hunt_guard">对沃玛护卫们见一个杀一个就能找到灵魂明珠。</event></par>
                        </layout>
                    ]=])
                end,

                npc_hunt_guard = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么……虽然很对不起你，但这次没有办法帮你了！因为我也没法儿知道灵魂明珠到底在哪个沃玛护卫手里拿着……</par>
                            <par>不管怎样，只要一找到灵魂明珠就马上拿给我。接下来的事儿到时候再作打算吧！</par>
                            <par>灵魂明珠在沃玛教主的部下沃玛护卫那儿。沃玛护卫要比一般的沃玛战士更加强大，所以一定要小心才是。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_orb'})
                end,
            }
        ]])
    end,

    -- hunting the guards
    quest_find_orb = function(uid, args)
        setQuestDesp{uid=uid, '灵魂明珠在沃玛护卫手里，去沃玛神殿猎杀沃玛护卫。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>灵魂明珠在哪儿？难道还没能找来吗？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    -- it will not break, and that points at the priests' hall
    quest_break_orb = function(uid, args)
        setQuestDesp{uid=uid, '取得了灵魂明珠，拿给无名老人。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交出灵魂明珠',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂明珠', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>灵魂明珠在哪儿？难道还没能找来吗？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>拿来灵魂明珠了？对，就是这个，辛苦了！</par>
                            <par>呃咳，呃咳，可怜的人们啊……马上就会让你们自由的……嗯？啊，没什么，我在自言自语呢。</par>
                            <par><event id="npc_ask_break">把这个毁掉就行吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_break = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>就是这个！我已经老了没有力气，你能来帮我把这个弄碎吗？</par>
                            <par><event id="npc_throw">扔到地上。</event></par>
                            <par><event id="npc_headbutt">用头碰撞。</event></par>
                            <par><event id="npc_smash">用武器向下打。</event></par>
                        </layout>
                    ]=])
                end,

                npc_throw = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唔……没有破啊！试试别的办法吧。</par>
                            <par><event id="npc_ask_break">后退</event></par>
                        </layout>
                    ]=])
                end,

                npc_headbutt = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>用一下此外别的办法，试试用头碰撞一下吧！</par>
                            <par><event id="npc_ask_break">后退</event></par>
                        </layout>
                    ]=])
                end,

                npc_smash = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还是丝毫未损！和预想中的差不多……先住手吧！</par>
                            <par><event id="npc_ask_how">怎样才能破坏它呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_how = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你也感觉到了吧！由于这个灵魂明珠是强大魔法的集结体，所以用一般的办法是绝对破坏不了的。而且为了里面被锁住的无数冤魂能够平安的升天，也不能用太勉强的办法。这样的话……呃咳，呃咳，既然解铃还需系铃人，那么也许到制造这个灵魂明珠的地方就能毁掉它呢！至少知道了这个办法，没准会有很大帮助呢！</par>
                            <par><event id="npc_go_hall">果然，也许会是那样吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_go_hall = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>过去沃玛教的祭司们居住的祭司馆在沃玛神殿3层，据我所知灵魂明珠就是在那儿制造的，也许那儿有破坏它的办法。虽然进入祭司馆需要用沃玛金牌，但是你已经找到了沃玛金牌，所以没什么问题。进去后就好好调查里面一下吧！</par>
                            <par>去调查一下在沃玛神殿3层的祭司馆吧！对了，不要忘了带上沃玛金牌！祭司馆在沃玛神殿2层的最北部附近，进去的入口被隐藏了，找起来可能有点难！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_meet_priest'})
                end,
            }
        ]])
    end,

    -- 牛老道 talks the orb straight out of your hands
    quest_meet_priest = function(uid, args)
        setQuestDesp{uid=uid, '带着沃玛金牌和灵魂明珠，去沃玛神殿3层的祭司馆查个究竟。'}

        setupNPCQuestBehavior(priestMap, '牛老道_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和牛老道说话',
                [SYS_ENTER] = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂明珠', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>难道您不知道灵魂明珠的去向吗？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>快请进。是带着灵魂明珠的人吧！</par>
                            <par><event id="npc_ask_who">您是谁？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_who = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我是道馆的道士。听说您在寻找破坏灵魂明珠的办法，所以特地赶来帮助你的。</par>
                            <par><event id="npc_i_see">哦……是这样啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_i_see = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那个灵魂明珠是非常危险的东西，我要把它带回道馆和众位道友用道力来净化一下。把那个灵魂明珠交给我吧！</par>
                            <par><event id="npc_hand_orb">好的。</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_orb = function(uid, args)
                    if not server.player.hasItem(uid, '灵魂明珠', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>好像您没有带着灵魂明珠啊！</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哈哈哈，嘿嘿嘿……得到灵魂明珠了！</par>
                            <par><event id="npc_tricked">啊！我被骗了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_tricked = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵呵…这个傻小子！真是不知天高地厚啊！看来是该让你知道一下想碰灵魂明珠要付出的代价了！</par>
                            <par>呵呵呵…弟兄们啊！收拾了这个家伙吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '灵魂明珠', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_fight_escort'})
                end,
            }
        ]])
    end,

    -- his 弟兄们 first
    quest_fight_escort = function(uid, args)
        setQuestDesp{uid=uid, '牛老道骗走了灵魂明珠，打退他召来的沃玛怪物。'}

        npcbattle.turnHostile
        {
            map     = priestMap,
            npc     = '牛老道_1',
            uid     = uid,
            monster = {'火焰沃玛', '沃玛战士', '沃玛战士'},
            x       = priestX,
            y       = priestY,
            say     = '牛老道大笑着退开，几只沃玛怪物从阴影里扑了上来！',
        }
    end,

    -- then he admits what he is
    quest_fight_priest = function(uid, args)
        setQuestDesp{uid=uid, '打退了牛老道的手下，再去和他理论。'}

        setupNPCQuestBehavior(priestMap, '牛老道_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和牛老道理论',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵呵…你这个不知天高地厚的人又来了啊！</par>
                            <par>作为人还是做得不错。</par>
                            <par><event id="npc_ask_human">难道你是说你不是人吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_human = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵呵…也曾经是过人……但在伟大的沃玛神赐予我新的不死之躯和强大力量之后，现在……我已经成为了超越人类的存在！！</par>
                            <par><event id="npc_challenge">说什么大话啊！我一定要亲手除掉你这个混蛋！</event></par>
                        </layout>
                    ]=])
                end,

                npc_challenge = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>呵呵呵……被我的力量吓跑的你又回来了啊！</par>
                            <par>嘿…让你见识一下超越人类的我的厉害！！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_kill_priest'})
                end,
            }
        ]])
    end,

    -- 牛老道 himself, and the hammer that made the orb
    quest_kill_priest = function(uid, args)
        setQuestDesp{uid=uid, '牛老道现出了原形，打倒他夺回灵魂明珠。'}

        npcbattle.turnHostile
        {
            map     = priestMap,
            npc     = '牛老道_1',
            uid     = uid,
            monster = '沃玛勇士',
            x       = priestX,
            y       = priestY,
            say     = '牛老道的皮囊裂了开来，里面是一头不死的沃玛怪物！',
        }
    end,

    -- the old man can finally do what he has waited decades for
    quest_smash_orb = function(uid, args)
        setQuestDesp{uid=uid, '夺回了灵魂明珠，还找到了沃玛神铁锤，把它们一起带给无名老人。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '交出灵魂明珠和沃玛神铁锤',
                [SYS_ENTER] = function(uid, args)
                    if not (server.player.hasItem(uid, '灵魂明珠', 1) and server.player.hasItem(uid, '沃玛神铁锤', 1)) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你说把灵魂明珠给了一个叫做牛老道的家伙？</par>
                                <par>知道那个地方的人除了我之外好像就没有别人了啊！牛老道……好像在哪儿听说过这个名字……</par>
                                <par>不管怎样赶快再去把灵魂明珠找回来，这可不是能随便就给陌生人的东西啊！</par>
                                <par>啊？灵魂明珠和沃玛神铁锤哪儿去了？那些可是重要的东西……</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>牛老道……啊，是那个家伙啊！错把沃玛教主当成沃玛神降临到地上的高位祭司们之中有个叫牛老道的人，中了沃玛教主的魔法变成了怪物。</par>
                            <par>不过幸运的是你把沃玛神铁锤找来了。可能这个就是制造灵魂明珠曾用过的工具。想起来好像很久以前曾经听说过从沃玛神殿中发掘出来了这个东西……</par>
                            <par>不管怎样用这个铁锤没准儿能破坏灵魂明珠呢……把灵魂明珠和铁锤拿到这儿来吧！</par>
                            <par><event id="npc_hand_all">给您。</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_all = function(uid, args)
                    if not (server.player.hasItem(uid, '灵魂明珠', 1) and server.player.hasItem(uid, '沃玛神铁锤', 1)) then
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哈哈，灵魂明珠开始裂开了！你感觉到被锁住的灵魂从这缝中徐徐地出来了吗？嗯？</par>
                            <par>呵呵呵呵呵呵……</par>
                            <par><event id="npc_wish_done">终于实现了夙愿啊！</event></par>
                        </layout>
                    ]=])

                    server.player.removeItem(uid, '沃玛金牌', 1)
                    server.player.removeItem(uid, '灵魂明珠', 1)
                    server.player.removeItem(uid, '沃玛神铁锤', 1)
                end,

                npc_wish_done = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这灵魂明珠中还有我弟弟的灵魂呢！现在弟弟能够升天到平安的世界中去了，我也就死而无憾了！</par>
                            <par><event id="npc_ask_brother">您弟弟也被邪教徒作为牺牲祭祀了啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_brother = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>确切地说，是死在我的手里。我……由于深陷这邪教之中把弟弟都作为活祭物献上了啊！无视他哭着被拖走的模样，幻想着只有我才能够得到主的洗礼……</par>
                            <par>我……我终究不能原谅我自己啊！直到现在也是一样。</par>
                            <par><event id="npc_absolve">可是老人家您现在已经完全赎罪了啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_absolve = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这还不够啊！因为沃玛教主仍然深藏在那沃玛神殿的深处的缘故啊！如果不在那个恶魔找到代替灵魂明珠唤醒他的方法之前除掉他的话，这种悲剧不知还会重新上演多少次呢！</par>
                            <par><event id="npc_take_job">那么就让我去除掉沃玛教主吧！</event></par>
                            <par><event id="npc_decline_job">既然知道那么您就去除掉沃玛教主吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_decline_job = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>.........</par>
                            <par>你已经为我做了足够的事情了，看来我这个老家伙太贪心了啊！这是辛苦费，请您收下吧！唉…沃玛教主的事儿该怎么办呢……</par>
                            <par>那么请慢走啊!</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, SYS_GOLDNAME, 30000)
                end,

                npc_take_job = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您的侠义心肠会救了世上的无数人啊！我这个老家伙也从您那儿得到了希望。希望您一定要除掉沃玛教主消除无数牺牲者的怨恨啊！</par>
                            <par>如果觉得一个人有点儿吃力的话，就和朋友们协力对付吧！一定要让沃玛教主死在<t color="red">%s</t>您和您的朋友手里啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)

                    server.player.addItem(uid, '修罗', 1)
                    server.player.addItem(uid, '偃月', 1)
                    server.player.addItem(uid, '降魔', 1)
                    server.quest.setState(questUID, {uid=uid, state='quest_kill_king'})
                end,
            }
        ]])
    end,

    -- the thing itself
    quest_kill_king = function(uid, args)
        setQuestDesp{uid=uid, '灵魂明珠已碎，去沃玛神殿深处除掉沃玛教主。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '和无名老人说话',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>希望你一定要让那个沃玛教主下地狱啊！</par>
                            <par>如果觉得一个人有点儿吃力的话，就和朋友们协力对付吧！一定要让沃玛教主死在<t color="red">%s</t>您和您的朋友手里啊！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], server.player.getName(uid), SYS_EXIT)
                end,
            }
        ]])
    end,

    -- the souls are free and so is he
    quest_king_dead = function(uid, args)
        setQuestDesp{uid=uid, '沃玛教主已除，回去告诉无名老人。'}

        setupNPCQuestBehavior(hermitMap, '无名老人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '告诉无名老人沃玛教主已除',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>....</par>
                            <par>嗯…呜…呜，对不起，流泪了啊，啊，眼泪都止不住了……</par>
                            <par><event id="npc_comfort">现在请您宽一下心吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_comfort = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>谢谢了！我死去的弟弟也不会忘记您的恩德的！真的太谢谢您能帮我这个罪孽深重的老家伙完成夙愿了！现在这个对我来说已经用不着了，如果您能够派上用场的话就好了。</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.addItem(uid, '沃玛修罗', 1)
                    server.player.addItem(uid, '沃玛偃月', 1)
                    server.player.addItem(uid, '沃玛降魔', 1)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

mondrop.setDropOnKill
{
    {
        monster  = '沃玛战士',
        state    = SYS_ENTER,
        kills    = 5,
        once     = true,
        give     = '沃玛金牌',
        setState = 'quest_sell_medal',
        say      = '（一面沉甸甸的金牌，纹样已经看不真切了）',
    },

    {
        monster = '沃玛卫士',
        state   = 'quest_find_hermit',
        kills   = 5,
        once    = true,
        give    = '无名日志',
        say     = '（一本被血浸透的旧日志，字迹还认得出来）',
    },

    {
        monster  = '沃玛护卫',
        state    = 'quest_find_orb',
        kills    = 5,
        give     = '灵魂明珠',
        setState = 'quest_break_orb',
        say      = '（一颗温热的明珠，里面像是有什么东西在动）',
    },

    -- his escort, then 牛老道 himself
    {
        monster  = '火焰沃玛',
        state    = 'quest_fight_escort',
        setState = 'quest_fight_priest',
        say      = '（看来需要再谈谈...）',
    },

    {
        monster  = '沃玛勇士',
        state    = 'quest_kill_priest',
        give     = {{'灵魂明珠', 1}, {'沃玛神铁锤', 1}},
        setState = 'quest_smash_orb',
        say      = '（灵魂明珠回来了，还有一柄沉重的古锤）',
    },

    {
        monster  = '沃玛教主',
        state    = 'quest_kill_king',
        setState = 'quest_king_dead',
        say      = '（冤魂们终于可以升天了……）',
    },
}

uidRemoteCall(getNPCharUID('道馆_1', '王铁匠_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>别来跟我说话……</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>喀儿喀儿，要在更晚之前打破这个恶业的羁绊。</par>
                    <par><event id="npc_ask_alone">您为什么独自一人在这儿生活呢？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_alone = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>为以前的过失赎罪啊……不管怎样这跟你无关。没有什么事儿的话你就走吧！我只不过是个无名的老头儿，不值得您操心！</par>
                    <par><event id="npc_ask_relic">您不卖沃玛神殿的古董吗？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_relic = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你是从哪儿听说我有跟沃玛神殿有关的东西的？嗯……如果是那个姓王的富人派来的人的话，还是请回吧！</par>
                    <par><event id="npc_ask_temple">您对沃玛神殿很了解吗？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_temple = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>怎么？你要亲自进去找“古董”？那个地方啊……简而言之就是个地狱，地狱啊！你难道是鸡脑子啊！简直是去那儿自寻死路啊！</par>
                    <par>还是别进沃玛神殿啦！为了贪图宝物可是会丢了性命的！我可是警告你了啊！</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
