function main()
    uidExecute(getNPCharUID('比奇县_0', '药剂师_1'),
    [[
        local questUID  = %d
        local questName = %s
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_CHECKACTIVE] = function(uid)
                return uidExecute(uid, [=[ return getQuestState('比奇商会') ]=]) == 'quest_persuade_pharmacist_and_librarian'
            end,

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
                        <par><event id="%%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)
                uidExecute(questUID, [=[ setUIDQuestState(%%d, SYS_ENTER) ]=], uid)
            end,

            npc_refuse = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>明白吗，年轻人？！</par>
                        <par>千万不要太拖延而忘了一切啊！人命关天啊！</par>
                        <par>我们所有人啊！</par>
                        <par></par>
                        <par><event id="%%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,
        })
    ]], getUID(), asInitString(getQuestName()))

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '药剂师_1'),
            [[
                local playerUID = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>请快去快回！</par>
                                <par></par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, asInitString(getQuestName()))
        end,
    })
end
