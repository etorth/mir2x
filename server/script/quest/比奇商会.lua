-- requires:
--      1. dbHasFlag('done_wang_book')
-- when done:
--      1. dbAddFlag('done_wang_coc')
--      2.

_G.fsmName_persuade_librarian  = '劝说图书管理人加入比奇商会'
_G.fsmName_persuade_pharmacist = '劝说药剂师加入比奇商会'

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, value)
        uidRemoteCall(getNPCharUID('比奇县_0', '王大人_1'), uid, value,
        [[
            local playerUID, accepted = ...
            if accepted then
                uidPostXML(playerUID,
                [=[
                    <layout>
                        <par>真的太感谢了！我期待着你能带来好消息！</par>
                        <par>传奇商会所属的其它商人仍然还有很多，但是现在凭我自己的力量很难一一说服。虽然从好几个方面同时下手。不管怎样？难道不该先避免沦为乞丐吗？所以拜托啦！</par>
                        <par></par>
                        <par><event id="%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)

            else
                uidPostXML(playerUID,
                [=[
                    <layout>
                        <par>不行？</par>
                        <par>那么我只好再去找其他人了。</par>
                        <par></par>
                        <par><event id="%s">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end
        ]])

        if value then
            setQuestState{uid=uid, state='quest_accept_quest'}
        else
            setQuestState{uid=uid, state='quest_refuse_quest'}
        end
    end,

    quest_accept_quest = function(uid, value)
        setQuestState{uid=uid, state='quest_persuade_pharmacist_and_librarian'}
    end,

    quest_refuse_quest = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那么现在可以帮助我了吗？情况紧急啊！</par>
                            <par></par>
                            <par><event id="npc_accept" close="1">好的</event></par>
                            <par><event id="npc_deny">我没有多余的精力</event></par>
                        </layout>
                    ]=])
                end,

                npc_accept = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, state='quest_accept_quest'}
                    ]=])
                end,

                npc_deny = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉！</par>
                            <par>老天爷啊！真的丢下我不管了吗！？</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    quest_persuade_pharmacist_and_librarian = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '王大人_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    local fsmState_pharmacist, fsmState_librarian = uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        return dbGetQuestState(playerUID, fsmName_persuade_pharmacist),
                               dbGetQuestState(playerUID, fsmName_persuade_librarian)
                    ]=])

                    local donePharmacist = (fsmState_pharmacist == SYS_DONE)
                    local doneLibrarian  = (fsmState_librarian  == SYS_DONE)

                    if (not donePharmacist) and (not doneLibrarian) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我期待着你能带来好消息！</par>
                                <par>只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！</par>
                                <par></par>
                                <par><event id="%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    elseif not doneLibrarian then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>还没能拉拢<t color="red">图书管理人</t>啊？再加把劲儿！</par>
                                <par>只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！</par>
                                <par></par>
                                <par><event id="%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    elseif not donePharmacist then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>还没能拉拢<t color="red">药剂师</t>啊？再加把劲儿！</par>
                                <par>只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！</par>
                                <par></par>
                                <par><event id="%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>噢！我们终于让<t color="red">图书管理人</t>和<t color="red">药剂师</t>从传奇商会退出了！</par>
                                <par>他们真的说同意加入比奇商会啦？做得好！做得实在是太棒啦！哈哈哈！</par>
                                <par></par>
                                <par><event id="npc_give_bonus">你过奖了！</event></par>
                            </layout>
                        ]=])
                    end
                end,

                npc_give_bonus = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真没想到啊！不知不觉中就把传奇商会的商家拉拢到我们这一方啦！真是手腕精明啊！由于你的活动终于使我们比奇商会统一了比奇地区商权。这是为了报答你的功劳准备的一点小小礼物，请不要谦让务必收下。</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        dbAddFlag('done_wang_coc')
                        setQuestState{uid=playerUID, state=SYS_DONE}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>早以前这里就是武林人士聚集的地方，呵呵。</par>
                            <par>你看起来也像个习武之人，来这里有什么事情吗？</par>
                            <par></par>
                            <par><event id="npc_discuss_1">我为了劝您加入比奇商会而来此的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_discuss_1 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>比奇商会？</par>
                            <par>啊！啊！知道了！是那个叫做王大人的创办的商人联合会吧！可是我已经加入了崔大夫创办的传奇商会，还是去别的地方试试吧！</par>
                            <par></par>
                            <par><event id="npc_discuss_2">也就是说无论如何都不行吗？</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_discuss_2 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>无论如何？那倒也不是。</par>
                            <par>如果你能为我办点事情的话，我也不是不能考虑加入比奇商会的。</par>
                            <par></par>
                            <par><event id="npc_discuss_3">要我帮你做什么事儿才行呢？</event></par>
                        </layout>
                    ]=])
                end,

                npc_discuss_3 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>其实最近我正在编撰记录比奇省地理和历史的书籍。如果想要写好这本书的话必然要从各种各样的人那里收集关于比奇省的资料和信息，可是唯独比奇省的卫士们那里不与我合作啊！</par>
                            <par>不管怎么样你也是武林人士，可能和他们能够有通融的地方，所以这就是我要拜托你的事情！值班卫士反正也不能和别人说话，所以希望你能替我去那儿找那些休班卫士从他们那里收集关于比奇省历史的故事。如果你能做到的话，我会听你的劝告加入比奇商会的。</par>
                            <par></par>
                            <par><event id="npc_accept">也许我可以去试试？</event></par>
                            <par><event id="%s">我和他们也不熟啊！</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦？你答应我的请求了？</par>
                            <par>那太好了，我等着你的好消息！</par>
                            <par></par>
                            <par><event id="%s">好的！</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state=SYS_ENTER}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我现在特别忙，你有什么事儿吗？</par>
                            <par></par>
                            <par><event id="npc_discuss_1">我来劝说你加入比奇商会！</event></par>
                        </layout>
                    ]=])
                end,

                npc_discuss_1 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>让我加入王大人的比奇商会？</par>
                            <par>你不知道我已经加入传奇商会了吗？呵呵，不过听说王大人那个人也不错，而且比起传奇商会来说条件也要更好。</par>
                            <par>但是不管怎么说都要讲点道义啊，怎么能像手心手背那样说翻就翻呢？</par>
                            <par></par>
                            <par><event id="npc_discuss_2">那就没有别的办法了吗？</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_discuss_2 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯！？既然你都这么说了，我倒是有一个建议。</par>
                            <par>最近比奇省里流行传染病，配制治疗这种病的药所需的原料毒蛇牙齿非常的紧缺。这种毒蛇牙齿在毒蛇山谷村就有卖的，但是我现在马上要给源源不断而来的病人治病，没有去买药材的时间。</par>
                            <par>传奇商会那帮人唯利是图，人命关天的事却无人愿意搭把手帮助我。如果你能够买来足够我们所需的毒蛇牙齿，我就会抛开商人的身份来以医生的角度听从您的劝说。</par>
                            <par></par>
                            <par><event id="npc_accept">没问题！</event></par>
                            <par><event id="npc_refuse">请给我点儿考虑的时间。</event></par>
                        </layout>
                    ]=])
                end,

                npc_accept = function(uid, value)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>从这儿向东北部去就能到达毒蛇山谷，可能去(643,15)附近就能够找得到。</par>
                            <par>穿过毒蛇山谷一直向东走就会达到那个村庄。在那儿找药商<t color="red">金中医</t>(334,224)向他购买<t color="red">毒蛇牙齿</t>。</par>
                            <par>现在患者数量仍然呈增加的趋势，所以还不能推测出以后具体需要多少药材。不管怎么样你都要快去快回。</par>
                            <par></par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state=SYS_ENTER}
                    ]=])
                end,

                npc_refuse = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>明白吗，年轻人？！</par>
                            <par>千万不要太拖延而忘了一切啊！人命关天啊！</par>
                            <par>我们所有人啊！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setQuestState{uid=uid, state='quest_wait_done'}
    end,

    quest_wait_done = function(uid, value)
        -- if not use this empty state
        -- then fsm stops in quest_persuade_pharmacist_and_librarian, and which setups npc behaviors
        -- but npc behavior is also been set in other fsm, it may overwrite
    end,
})

setQuestFSMTable(fsmName_persuade_librarian,
{
    [SYS_ENTER] = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
        [[
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>你看起来还没有听到休班卫士的全部故事啊？可以在比奇省内转转就可以找到休班卫士！</par>
                            <par></par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这鬼天气！真是让人心烦气躁，要是能来口酒润润嗓子该多好啊！</par>
                            <par>随便打扰别人真是没礼貌，有什么事情？</par>
                            <par></par>
                            <par><event id="npc_ask_guard_1_info">你是否知道比奇省的历史？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_guard_1_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗨！你这个没教养的家伙!求别人办事情至少要应该有点诚意吧？真是不明事理啊！</par>
                            <par>唔, 嗓子有点干，想去酒店喝杯酒啊！咦？这个月的薪水已经全都喝酒花干净了！钱可真不经花啊！</par>
                            <par></par>
                            <par><event id="npc_guard_1_wait_soju" close="1">退出</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_1_wait_soju = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_give_guard_1_soju'}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_2', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    if uidRemoteCall(questUID, uid, [=[ return hasQuestFlag(..., 'flag_done_query_guard_2') ]=]) then
                        runEventHandler(uid, questPath, 'npc_guard_2_deny')
                    else
                        runEventHandler(uid, questPath, 'npc_guard_2_accept')
                    end
                end,

                npc_guard_2_accept = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>听说有人找我，是你吗？</par>
                            <par>鄙人就是崔某，有什么事儿吗？</par>
                            <par></par>
                            <par><event id="npc_guard_2_give_info">我想知道有关比奇省的历史。</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_2_deny = function(uid, value)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>为什么还要再来？</par>
                            <par>我已经把我知道的都告诉你了！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_guard_2_give_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>说起比奇省的历史...</par>
                            <par>知道吗？我们的祖先就是讨伐半兽人族地区而派遣出的远征队啊！我们的祖先经过残酷的战斗终于击溃了怪物们。一想到只要再继续坚持战斗一下就可以把怪物们斩草除根，然后可以回到故乡，就都非常高兴。</par>
                            <par>可是没想到这时突然发生了始料未及的灾难。这里发生了大地震。原本可以翻过山脉回到家乡的路由于这次大地震导致地壳变动，完全的被隔断了！有的人痛哭流涕，有的人茫然失措。所有人都慌了手脚。</par>
                            <par>但是一位优秀的将领重新振作精神，开始在这个地区寻找求生之路。他指挥着他的部下们在赶走半兽人族的地区找到了一片肥沃的土地建立了新的城市。这就是现在的比奇省。</par>
                            <par>好了，我已经把知道的基本上全都告诉你啦...我也要走啦！</par>
                            <par></par>
                            <par><event id="npc_done_query_guard_2" close="1">谢谢！</event></par>
                        </layout>
                    ]=])
                end,

                npc_done_query_guard_2 = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        addQuestFlag(playerUID, 'flag_done_query_guard_2')
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_wait_guard_1_and_guard_2_done'}
                    ]=])
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是你啊？四处打听比奇省历史的人？</par>
                            <par></par>
                            <par><event id="npc_ask_guard_3_info">是的啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_guard_3_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！你也是来问我关于比奇省历史的吗？</par>
                            <par></par>
                            <par><event id="npc_guard_3_deny">是的, 请您讲讲比奇省历史的故事吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_deny = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>喂！我可是卫士中资历最深的！你先去跟其他的人打听之后再来找我吧！</par>
                            <par>不能让人小瞧了我...</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,

    quest_give_guard_1_soju = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    if uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 1) ]=]) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>啊！又是你，你能不能离我远...等等！这是烧酒的味道，好香啊！</par>
                                <par>这位%s，能不能给我喝口酒啊！</par>
                                <par></par>
                                <par><event id="npc_give_soju">拿去吧！</event></par>
                                <par><event id="npc_deny_soju">不愿意。</event></par>
                            </layout>
                        ]=], uidRemoteCall(uid, [=[ return getGender() ]=]) and '少侠' or '姑娘')
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>啊！又是你，你能不能离我远点！？大热天的为什么要三番五次地惹人烦呢？</par>
                                <par>要是能有口酒润润嗓子就好啦！</par>
                                <par></par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end,

                npc_give_soju = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊哈哈哈，真是谢谢你！</par>
                            <par>咕噜咕噜，还是烧酒的味道棒啊！咕噜咕噜，炎热的午后能痛快地喝一顿真是神仙般的快活啊！</par>
                            <par></par>
                            <par><event id="npc_ask_info">不知道你能否告知比奇省的历史？</event></par>
                        </layout>
                    ]=])
                end,

                npc_deny_soju = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你是不愿意给我喝呢，还是酒已经被喝光了？</par>
                            <par>真扫兴！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_ask_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>比奇省的历史？嗯？唔...</par>
                            <par>说起比奇的由来这要追溯到几百年之前啦！比奇产生之前，西方有几个国家，由于被叫做内日和半兽人的怪物种族袭击一直都处于危险之中，处于威机之中的这几个国家停止了相互之间的战争，协力与怪物们抗争，最后终于赶走了怪物们，但是也全部受到了重创，怪物们的威胁仍然没有完全解除。</par>
                            <par>于是这几个国家协力出兵去讨伐怪物们的根据地，那个地方就是比奇地区！</par>
                            <par>这些够了吧！</par>
                            <par></par>
                            <par><event id="npc_ask_more_info">你还知道更多的信息吗？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_more_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>看在烧酒的面子上，我知道的就这些啦！想知道的更多，你也可以去问问其他的卫士。</par>
                            <par></par>
                            <par><event id="npc_done_query_guard_1" close="1">好的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_done_query_guard_1 = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        addQuestFlag(playerUID, 'flag_done_query_guard_1')
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_wait_guard_1_and_guard_2_done'}
                    ]=])
                end,
            }
        ]])
    end,

    quest_wait_guard_1_and_guard_2_done = function(uid, value)
        local done_guard_1 = hasQuestFlag(uid, 'flag_done_query_guard_1')
        local done_guard_2 = hasQuestFlag(uid, 'flag_done_query_guard_2')

        if not (done_guard_1 and done_guard_2) then
            return
        end

        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>是你啊？四处打听比奇省历史的人？</par>
                            <par></par>
                            <par><event id="npc_ask_guard_3_info">是的啊！</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask_guard_3_info = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！你也是来问我关于比奇省历史的吗？</par>
                            <par></par>
                            <par><event id="npc_guard_3_answer">是的, 请您讲讲比奇省历史的故事吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_answer = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这事儿可就说来话长了...</par>
                            <par>噢！我可是什么都不知道！呵呵，你还是去问别人吧！</par>
                            <par></par>
                            <par><event id="npc_give_guard_3_gold" args="100"  close="1">给他100金币</event></par>
                            <par><event id="npc_give_guard_3_gold" args="1000" close="1">给他1000金币</event></par>
                            <par><event id="%s">不询问他</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_give_guard_3_gold = function(uid, value)
                    uidRemoteCall(questUID, uid, value, questName,
                    [=[
                        local playerUID, giveGold, questName = ...
                        local nextState = string.format('quest_give_guard_3_%s_gold', giveGold)

                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state=nextState, exitfunc=function()
                            runNPCEventHandler(getNPCharUID('比奇县_0', '休班卫士_3'), playerUID, {SYS_EPUID, questName}, SYS_ENTER)
                        end}
                    ]=])
                end,
            }
        ]])
    end,

    quest_give_guard_3_100_gold = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗨哎！？这是干吗？</par>
                            <par></par>
                            <par><event id="npc_guard_3_give_info">请以后买点酒喝什么的吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_give_info = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    {
                        [=[<par>哦？是嘛，哈哈哈！好吧，我来讲给你听。</par>]=],
                        [=[<par>唔...这已经是我所知道的全部故事啦！</par>]=],
                    },
                    [=[
                        local playerUID, texts = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_guard_3_give_info', args=texts}
                    ]=])
                end,
            }
        ]])
    end,

    quest_give_guard_3_1000_gold = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你...你这是做什么？竟敢和保护比奇省治安的我开这种玩笑？</par>
                            <par>看来和你是做不了朋友了！要和我比试比试吗？我长这么大还是头一次受到这种污辱！</par>
                            <par></par>
                            <par><event id="npc_guard_3_angry_1">你千万别误会啊！不是这个意思！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_angry_1 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你还狡辩什么啊？你这个%s！</par>
                            <par></par>
                            <par><event id="npc_guard_3_angry_2">你千万不要误会呀！</event></par>
                        </layout>
                    ]=], uidRemoteCall(uid, [=[ return getGender() ]=]) and '混小子' or '混丫头')
                end,

                npc_guard_3_angry_2 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哼！呵呵...没有别的意思！真的吗？</par>
                            <par></par>
                            <par><event id="npc_guard_3_angry_3">对不起是我错了，请原谅！</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_angry_3 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唉！没办法，谁让我年纪大来着呢，原谅一次你吧！这里有1个金币！</par>
                            <par>快去买<t color="red">5瓶烧酒</t>来，喝了酒才能消了我的肚子里的火气。别忘了把找还的零钱带回来！</par>
                            <par></par>
                            <par><event id="npc_guard_3_angry_4" close="1">好吧...</event></par>
                        </layout>
                    ]=])
                end,

                npc_guard_3_angry_4 = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_give_guard_3_soju'}
                    ]=])
                end,
            }
        ]])
    end,

    quest_give_guard_3_soju = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '休班卫士_3', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    local hasSoju    = uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 1) ]=])
                    local hasSoju_x5 = uidRemoteCall(uid, [=[ return hasItem(getItemID('烧酒'), 0, 5) ]=])

                    if not hasSoju then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>快去买啊！1个金币还不够吗？</par>
                                <par></par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    elseif not hasSoju_x5 then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>臭小子，我是说五瓶，快去再买点！</par>
                                <par>竟敢不听我的！</par>
                                <par></par>
                                <par><event id="%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>呵！你还真买来了。</par>
                                <par>咕噜，咕噜，啊！真是好酒，现在舒服多了。</par>
                                <par>对了，你是找我来问什么的来着？</par>
                                <par></par>
                                <par><event id="npc_guard_3_give_info">我想知道关于比奇省历史的事。</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end,

                npc_guard_3_give_info = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    {
                        [=[<par>好吧，我来讲给你听。</par>]=],
                        [=[<par>唔...这已经是我所知道的全部故事啦！就说到这里吧，酒喝得很爽啊！</par>]=],
                    },
                    [=[
                        local playerUID, texts = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state='quest_guard_3_give_info', args=texts}
                    ]=])
                end,
            }
        ]])
    end,

    quest_guard_3_give_info = function(uid, value)
        uidRemoteCall(getNPCharUID('比奇县_0', '休班卫士_3'), uid, value,
        [[
            local playerUID, texts = ...

            local text1 = texts[1] or ''
            local text2 = texts[2] or ''

            uidPostXML(playerUID,
            [=[
                <layout>
                    %s
                    <par>祖先们修建了这比奇省和里面的城镇村庄之后，就开始反复的在周边勘查并拓展自己的根据地。但是这附近值得利用的土地非常的少。很难足够的支持别的地方的农事生产需要。</par>
                    <par>随着人口逐渐的增加，人们为了寻找更加宽阔的土地和更多的资源开始拓宽自己的领土。于是人们向沃玛、蛇谷、盟众一步一步的扩大土地，开拓没有人烟到达过的沼泽地，也遇到了生活在森林、灌木丛和山洞中其它各种各样的怪物并与它们发生战争，就这样一点一点的扩大了领土，可以说每一寸土地都是用鲜血换来的啊！</par>
                    <par>尽管我们现在占据了宽广的领土，但在比奇土地上各处都仍存在着怪物的势力，加上大部分地区全都是深山和茂密的灌木丛，仍然会发生种种阻断村庄之间道路的事情...</par>
                    %s
                    <par></par>
                    <par><event id="%s">谢谢你！</event></par>
                </layout>
            ]=], text1, text2, SYS_EXIT)
        ]])

        setQuestState{uid=uid, fsm=fsmName_persuade_librarian, state='quest_answer_librarian_questions'}
    end,

    quest_answer_librarian_questions = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>听说你已经收集到了所有关于比奇省的历史了？真是太好了！</par>
                            <par></par>
                            <par><event id="npc_question_1">是的</event></par>
                        </layout>
                    ]=])
                end,

                npc_question_1 = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我有个问题不确定很久了，正好问你！</par>
                            <par>听说在比奇产生之前，西方的国家为了讨伐某种怪物派种族遣出过远征队，那种怪物是什么种族的呢？</par>
                            <par></par>
                            <par><event id="npc_question_2" args="1">诺玛和内日</event></par>
                            <par><event id="npc_question_2" args="2">沃玛和祖玛</event></par>
                            <par><event id="npc_question_2" args="3">半兽人和内日</event></par>
                            <par><event id="npc_question_2" args="4">半兽人和诺玛</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_question_2 = function(uid, value)
                    if value == '3' then
                        local selections = shuffleArray(
                        {
                            [=[ <par><event id="npc_question_3" args="1">祖玛教主的宫殿</event></par> ]=],
                            [=[ <par><event id="npc_question_3" args="2">蛇的集体栖息地</event></par> ]=],
                            [=[ <par><event id="npc_question_3" args="3">半兽人的根据地</event></par> ]=],
                            [=[ <par><event id="npc_question_3" args="4">内日族的根据地</event></par> ]=],
                        })

                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>哦！原来还是那些家伙啊...</par>
                                <par>那么在人类来此生活之前的比奇县什么样的地方呢？</par>
                                <par></par>
                                %s
                                %s
                                %s
                                %s
                            </layout>
                        ]=], selections[1], selections[2], selections[3], selections[4])
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                npc_question_3 = function(uid, value)
                    if value == '3' then
                        local selections = shuffleArray(
                        {
                            [=[ <par><event id="npc_done_question" args="1">因为发生了大地震</event></par> ]=],
                            [=[ <par><event id="npc_done_question" args="2">因为发生了大洪水</event></par> ]=],
                            [=[ <par><event id="npc_done_question" args="3">因为发生了大饥荒</event></par> ]=],
                            [=[ <par><event id="npc_done_question" args="4">因为发生了大瘟疫</event></par> ]=],
                        })

                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>果然！难怪半兽人那么顽固地反扑...</par>
                                <par>那么我们的祖先们回不了故乡，在这个地方落脚定居的原因是什么呢？</par>
                                <par></par>
                                %s
                                %s
                                %s
                                %s
                            </layout>
                        ]=], selections[1], selections[2], selections[3], selections[4])
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                npc_done_question = function(uid, value)
                    if value == "1" then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>怪不得呢！所以越过山脉的路就被隔断了啊！</par>
                                <par>你真的是认真努力的调查过啦！托您的福，史书的撰写进度加快了！等这本书全部完成之后一定会在末尾写上你的大名的。</par>
                                <par>那么请你去把我要加入比奇商会的意思转告给王大人吧！</par>
                                <par></par>
                                <par>真是太谢谢了！</par>
                            </layout>
                        ]=])

                        uidRemoteCall(questUID, uid, getNPCMapName(false), getNPCName(false),
                        [=[
                            local playerUID, mapName, npcName = ...
                            clearNPCQuestBehavior(mapName, npcName, playerUID)
                            setQuestState{uid=playerUID, fsm=fsmName_persuade_librarian, state=SYS_DONE}
                        ]=])
                    else
                        runEventHandler(uid, questPath, 'npc_wrong_answer')
                    end
                end,

                npc_wrong_answer = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>奇怪！根据我的调查好像不是这么回事儿啊！你确定没有听错吗？</par>
                            <par>请再去打听一下吧！</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,
})

setQuestFSMTable(fsmName_persuade_pharmacist,
{
    [SYS_ENTER] = function(uid, value)
        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>患者越来越多，快去<event id="npc_path_details">毒蛇山谷</event>买<t color="red">毒蛇牙齿</t>吧！</par>
                            <par></par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_path_details = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>从这儿向东北部去就能到达毒蛇山谷，去(643,15)附近就能够找得到。穿过毒蛇山谷一直向东走就会达到那个村庄，在那儿找药商<t color="red">金中医</t>(334,224)向他购买<t color="red">毒蛇牙齿</t>。</par>
                            <par></par>
                            <par><event id="%s">好的</event></par>
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
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>噢！是来买毒蛇牙齿的啊！</par>
                            <par></par>
                            <par><event id="npc_buy_tooth">是的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_buy_tooth = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>唔？看起来你不是要药材商或者从医的人吧！</par>
                            <par>这倒无所谓！不过作为药用的毒蛇牙齿的产量是固定的，每天充其量能供给几包而已。难道你不知道吗？</par>
                            <par></par>
                            <par><event id="npc_seller_call_price">我是受比奇省药剂师之托而来的</event></par>
                        </layout>
                    ]=])
                end,

                npc_seller_call_price = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>比奇省发生传染病? 你说的是真的吗？那我现在就卖给你一包吧！</par>
                            <par>现在只有这些，如果需要的话再来吧！价格是100钱一颗，给我1000钱就行。</par>
                            <par><event id="npc_pay_full_price">全额付款</event></par>
                            <par><event id="npc_ask_for_discount">讨价还价</event></par>
                        </layout>
                    ]=])
                end,

                npc_pay_full_price = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>很着急的样子啊！</par>
                            <par>给你，快去比奇省看看吧！</par>
                            <par></par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_purchased_tooth'}
                    ]=])
                end,

                npc_ask_for_discount = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>城内的情况这么紧急的话我就赔本卖给你吧！</par>
                            <par>每颗10钱，只付100钱就行。连加工费都去掉就按成本给你啦！</par>
                            <par></par>
                            <par><event id="npc_setup_purchase_quest">好的</event></par>
                        </layout>
                    ]=])
                end,

                npc_setup_purchase_quest = function(uid, value)
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_purchase_with_agreed_price', args=100}
                    ]=])
                end,
            }
        ]])
    end,

    quest_purchase_with_agreed_price = function(uid, value)

        -- purchase with agreed price
        -- this state won't pop up dialog if player has enough money

        assertType(value, 'integer')
        assert(value > 0)

        uidRemoteCall(getNPCharUID('毒蛇山谷_2', '金中医_1'), uid, value, getUID(), getQuestName(),
        [[
            local playerUID, askedGold, questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local currGold = uidRemoteCall(playerUID, [=[ return getGold() ]=])

            if currGold >= askedGold then
                uidPostXML(playerUID,
                [=[
                    <layout>
                        <par>东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！</par>
                        <par><event id="%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)

                uidRemoteCall(playerUID, askedGold,
                [=[
                    local askedGold = ...
                    removeItem(getItemID(SYS_GOLDNAME), 0, askedGold)
                    addItem(getItemID('毒蛇牙齿'), 10)
                ]=])

                uidRemoteCall(questUID, playerUID,
                [=[
                    local playerUID = ...
                    setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_purchased_tooth'}
                ]=])

            else
                local rand = math.random(0, 100)
                if rand <= 0 then
                    uidRemoteCall(questUID, playerUID, questPath,
                    [=[
                        local playerUID, questPath = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_purchase_with_free_price', exitfunc=function()
                            runEventHandler(playerUID, questPath, SYS_ENTER)
                        end}
                    ]=])

                elseif rand <= 50 then
                    uidPostXML(playerUID,
                    [=[
                        <layout>
                            <par>你是在开玩笑吗？你没有<t color="red">%d</t>金币啊？！</par>
                            <par>你这个不老实的家伙，不要再浪费我的时间了！先凑够<t color="red">%d</t>金币再来找我吧！</par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], askedGold, askedGold, SYS_EXIT)

                    uidRemoteCall(questUID, playerUID, askedGold,
                    [=[
                        local playerUID, askedGold = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_wait_purchase', args=askedGold}
                    ]=])

                else
                    local newAskedGold = math.ceil(askedGold * 1.5)
                    uidPostXML(playerUID,
                    [=[
                        <layout>
                            <par>你是在开玩笑吗？你没有<t color="red">%d</t>金币啊？！</par>
                            <par>你这个家伙实在浪费我的一片好心，不要再说了！我决定涨价<t color="red">50%%</t>，先凑够<t color="red">%d</t>金币再来找我吧！</par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], askedGold, newAskedGold, SYS_EXIT)

                    uidRemoteCall(questUID, playerUID, newAskedGold,
                    [=[
                        local playerUID, newAskedGold = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_wait_purchase', args=newAskedGold}
                    ]=])
                end
            end
        ]])
    end,

    quest_wait_purchase = function(uid, value)
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid, string.format([[ return %d, getUID(), getQuestName() ]], value),
        [[
            local askedGold, questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你带来<t color="red">%d</t>金币了吗？</par>
                            <par></par>
                            <par><event id="npc_purchase">带来了！</event></par>
                            <par><event id="%s">我还没凑齐！</event></par>
                        </layout>
                    ]=], askedGold, SYS_EXIT)
                end,

                npc_purchase = function(uid, value)
                    uidRemoteCall(questUID, uid, askedGold,
                    [=[
                        local playerUID, askedGold = ...
                        setQuestState{uid=playeUID, fsm=fsmName_persuade_pharmacist, state='quest_purchase_with_agreed_price', args=askedGold}
                    ]=])
                end,
            }
        ]])
    end,

    quest_purchase_with_free_price = function(uid, value)
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
        [[
            getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>真是个难缠的家伙！拿你没办法，先免费给你快拿去给病人们治病用吧！</par>
                            <par></par>
                            <par><event id="npc_say_thanks">谢谢你的慷慨！</event></par>
                        </layout>
                    ]=])
                end,

                npc_say_thanks = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我是看药剂师的面子才免费的！东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！</par>
                            <par><event id="%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(uid, [=[ addItem(getItemID('毒蛇牙齿'), 10) ]=])
                    uidRemoteCall(questUID, uid,
                    [=[
                        local playerUID = ...
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state='quest_purchased_tooth'}
                    ]=])
                end,
            }
        ]])
    end,

    quest_purchased_tooth = function(uid, value)
        setupNPCQuestBehavior('毒蛇山谷_2', '金中医_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>干嘛呢？还不快把药材带给<t color="yellow">比奇省</t><t color="red">药剂师</t>。</par>
                            <par></par>
                            <par><event id="%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '药剂师_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPQST, questName}

            return
            {
                [SYS_ENTER] = function(uid, value)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您为病人们做了一件大好事！所以我会听从你的劝说加入王大人的比奇商会的，只好对不起崔大夫了！</par>
                            <par>啊！对了，这是金创药，收下这个吧！急匆匆地走了这么远的路累坏了吧！喝了这个可以补充一下元气。</par>
                            <par></par>
                            <par><event id="%s">谢谢！</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidRemoteCall(uid, [=[ addItem(getItemID('金创药（特）'), 8) ]=])
                    uidRemoteCall(questUID, uid, getNPCMapName(false), getNPCName(false),
                    [=[
                        local playerUID, mapName, npcName = ...
                        clearNPCQuestBehavior(mapName, npcName, playerUID)
                        setQuestState{uid=playerUID, fsm=fsmName_persuade_pharmacist, state=SYS_DONE}
                    ]=])
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '王大人_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return SYS_DEBUG or uidRemoteCall(uid, [=[ return dbHasFlag('done_wang_book') ]=])
        end,

        [SYS_ENTER] = function(uid, value)
            local level = uidRemoteCall(uid, [=[ return getLevel() ]=])
            if level < 9 then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我要托付你帮我办点事情，但是现在的你好象还有点应付不了。再去好好修炼一下，等级达到9级的时候才能够得到我的信任让我把这件事情交给你去做。</par>
                        <par></par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            else
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>噢！你来啦！你要是不来我正好要派人去叫你呢？其实我是有事情要拜托你做！是这样的！最近以我为会长的比奇商会和以那个姓崔的贪得无厌的家伙为首的传奇商会正在互相争夺势力。</par>
                        <par>啊！当然不是真刀真枪的动武啦！是为了争夺商权而展开的势力之争。不管怎么样，胜者为王败者寇，在这场斗争中失败者将被排挤出比奇省。因此想要求你帮我办点事情，为了增强我们比奇商会的势力，请你去说服<t color="red">图书管理人</t>和施药商<t color="red">药剂师</t>从传奇商会中退出，来加入我们比奇商会。</par>
                        <par></par>
                        <par><event id="npc_decide" args="true" >好的！</event></par>
                        <par><event id="npc_decide" args="false">我还有别的事情要做。</event></par>
                    </layout>
                ]=])
            end
        end,

        npc_decide = function(uid, value)
            uidRemoteCall(questUID, uid, value,
            [=[
                local playerUID, accepted = ...
                setQuestState{uid=playerUID, state=SYS_ENTER, args=accepted}
            ]=])
        end,
    })
]])
