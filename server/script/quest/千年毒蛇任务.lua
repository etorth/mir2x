-- converted from Envir/QuestDiary/NQ_BASE/bamgol.txt
--
-- 蛇谷老太's granddaughter 珍珍 was bitten by something no ordinary antidote touches, 金中医
-- works out it must be the legendary 千年毒蛇 and that only its gall will do

_G.minQuestLevel = 16
_G.prequestName  = '被盗灵魂任务'

local mondrop = require('quest.include.mondrop')

setQuestFSMTable(
{
    -- accepted, but nobody knows yet what bit her
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '答应了蛇谷老太要为珍珍找解药，先去找毒蛇山谷的金中医问问是什么毒吧。'}

        setupNPCQuestBehavior('毒蛇山谷_2', '蛇谷老太_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '珍珍的解药',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有找到我们珍珍的药啊？</par>
                            <par>天哪！珍珍啊……珍珍……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问珍珍的蛇毒',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你说解蛇毒的药？啊！是说珍珍那孩子吧！</par>
                            <par>我也在为了寻找治疗这孩子的药而四处奔波，可是却一直没能找到。</par>
                            <par>其实在这蛇儿们聚集的毒蛇山谷建下了村庄而生活的我们一直认为蛇毒没什么大不了的。</par>
                            <par>然而看起来咬了珍珍的蛇好像不是普通的蛇啊！</par>
                            <par><event id="npc_ask_snake_kind">毒蛇山谷都有什么样的蛇呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_snake_kind = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>的确如此！</par>
                            <par>如果是被其中一种咬了的话，我也不至于无法解毒啊！但是珍珍所中的毒诱发了一种我从来没见过的罕见症状。毒性非常之剧就连用最好的解毒药都一点效果也没有。</par>
                            <par>唉……现在几乎所有可用的办法都用过了，唯一的一点希望就只剩最后的一种方法了！</par>
                            <par><event id="npc_ask_last_hope">是什么呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_last_hope = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>就是传说中<t color="red">千年毒蛇</t>的胆汁！这种蛇通身雪白，只有北斗七星形状的黑色花纹，被称作“万蛇之王”。此种蛇非常之少见，而且毒性也是剧毒无比，不过据说这千年毒蛇的胆汁可以解所有的蛇毒。</par>
                            <par>尽管不知道这世界上是否真的有这种叫作千年毒蛇的蛇，但是种种迹象都值得怀疑，咬了珍珍的好像就是这种千年毒蛇！</par>
                            <par>如果真的是这样的话，那么这千年毒蛇就应该存在于这毒蛇山谷之中的某个地方，那么这蛇胆不也就能够弄到手吗？</par>
                            <par><event id="npc_hunt_snake">那么抓到千年毒蛇弄到他的胆就行了！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hunt_snake = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那当然了……我现在也一直在找……可是还没能找到！</par>
                            <par>你也帮我一起找找吧！</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.quest.setState(questUID, {uid=uid, state='quest_find_gall'})
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
                            <par>珍珍中的毒可不一般，毒蛇山谷的金中医见过的蛇毒最多，去问问他吧！</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    -- knows what to hunt, the gall comes off a 千年毒蛇 corpse
    quest_find_gall = function(uid, args)
        setQuestDesp{uid=uid, '金中医说只有传说中千年毒蛇的胆汁才能解珍珍的毒，去毒蛇山谷猎杀千年毒蛇吧。'}

        setupNPCQuestBehavior('毒蛇山谷_2', '蛇谷老太_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '珍珍的解药',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有找到我们珍珍的药啊？</par>
                            <par>天哪！珍珍啊……珍珍……</par>
                            <par><event id="npc_tell_doctor">金中医说只有千年毒蛇的胆汁才行……</event></par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_tell_doctor = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦？金中医也说没有别的办法了？不是吧？嗯？</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问珍珍的蛇毒',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>找到千年毒蛇胆汁的话，就可以为珍珍解毒了！</par>
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
                            <par>通身雪白、带北斗七星黑纹的千年毒蛇就藏在毒蛇山谷里，取到它的胆汁就能救珍珍了。</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    -- carrying the gall, hand it to 蛇谷老太
    quest_got_gall = function(uid, args)
        setQuestDesp{uid=uid, '取得了千年毒蛇胆汁，快把它送给蛇谷老太吧。'}

        setupNPCQuestBehavior('毒蛇山谷_2', '蛇谷老太_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '送上千年毒蛇胆汁',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊！你说给我们的珍珍把药找来了？</par>
                            <par>金中医说服了这个就能救回珍珍的命？呜呜，真是太感谢你了！我一辈子都不会忘了你这个年轻人的恩情的！我会日夜为你这个年轻人祈祷祝你好运的！</par>
                            <par><event id="npc_hand_gall">还是赶快去让珍珍服下这药吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_hand_gall = function(uid, args)
                    if not server.player.hasItem(uid, '千年毒蛇胆汁', 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>咦？你说的药呢？</par>
                                <par><event id="%s" close="1">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        return
                    end

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>好的！真是太感谢了……</par>
                            <par>这个镯子是我的一点儿心意……</par>
                            <par><event id="%s" close="1">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    server.player.removeItem(uid, '千年毒蛇胆汁', 1)
                    server.player.addItem(uid, '波纹手镯', 1)
                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])

        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_LABEL] = '询问珍珍的蛇毒',
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哇！原来真的有这种传说中的千年毒蛇啊！</par>
                            <par>啊！现在还不是说这个的时候……赶快去把这个蛇胆给珍珍的奶奶送去吧！</par>
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
                            <par>蛇胆已经到手了，别再耽误，快给蛇谷老太送去！</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,
})

-- 千年毒蛇胆汁 is not in the monster drop table on purpose, it only comes off the corpse
-- once 金中医 has explained what to look for
mondrop.setDropOnKill
{
    {
        monster  = '千年毒蛇',
        state    = 'quest_find_gall',
        give     = '千年毒蛇胆汁',
        setState = 'quest_got_gall',
        say      = '你剖开千年毒蛇的尸体，取得了千年毒蛇胆汁！',
    },
}

uidRemoteCall(getNPCharUID('毒蛇山谷_2', '蛇谷老太_1'), getUID(), getQuestName(), minQuestLevel, prequestName,
[[
    local questUID, questName, minQuestLevel, prequestName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if server.player.getLevel(uid) < minQuestLevel or server.player.getQuestState(uid, prequestName) ~= SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>没有看到我的珍珍吗？这孩子到底跑哪儿去了？</par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>喂！年轻人！</par>
                    <par>救人一命胜造七级浮屠，帮我这个老人一个忙吧！</par>
                    <par><event id="npc_ask_favor">您有什么事儿吗？</event></par>
                </layout>
            ]=])
        end,

        npc_ask_favor = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不久前我孙女儿被蛇给咬了。</par>
                    <par>可是不知道是被什么蛇给咬了，什么解毒药都不好使啊！虽然现在找了非常贵的药草使病状不再恶化，但不知道还能维持多久……</par>
                    <par>一定要帮老人家我的小孙女儿找来药啊！</par>
                    <par><event id="npc_accept_quest">好的！</event></par>
                    <par><event id="npc_refuse_quest">我现在还有别的事儿……</event></par>
                </layout>
            ]=])
        end,

        npc_accept_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>谢谢你能帮助我啊！</par>
                    <par>真的非常感谢……</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_refuse_quest = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你说不行啊？</par>
                    <par>这可怎么办啊……</par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])
