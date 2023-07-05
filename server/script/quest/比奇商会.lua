function main()
    uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
    [[
        local questUID  = %d
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_CHECKACTIVE] = function(uid)
                return uidExecute(uid, [=[ return getQuestState('初出江湖') ]=]) == SYS_EXIT
            end,

            [SYS_ENTER] = function(uid, value)
                local level = uidExecute(uid, [=[ return getLevel() ]=])
                if level < 9 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>我要托付你帮我办点事情，但是现在的你好象还有点应付不了。再去好好修炼一下，等级达到9级的时候才能够得到我的信任让我把这件事情交给你去做。</par>
                            <par></par>
                            <par><event id="%%s">退出</event></par>
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
                uidExecute(questUID, [=[ setUIDQuestState(%%d, SYS_ENTER, %%s) ]=], uid, value)
            end,
        })
    ]], getUID(), getQuestName())

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
            [[
                local playerUID = %d
                local accepted  = %s
                local questUID  = %d

                if accepted then
                    uidPostXML(playerUID,
                    [=[
                        <layout>
                            <par>真的太感谢了！我期待着你能带来好消息！</par>
                            <par>传奇商会所属的其它商人仍然还有很多，但是现在凭我自己的力量很难一一说服。虽然从好几个方面同时下手。不管怎样？难道不该先避免沦为乞丐吗？所以拜托啦！</par>
                            <par></par>
                            <par><event id="%%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_accept_quest') ]=], playerUID)
                else
                    uidPostXML(playerUID,
                    [=[
                        <layout>
                            <par>不行？</par>
                            <par>那么我只好再去找其他人了。</par>
                            <par></par>
                            <par><event id="%%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                    uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_refuse_quest') ]=], playerUID)
                end
            ]], uid, asInitString(value), getUID())
        end,

        quest_accept_quest = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
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
                                <par>我期待着你能带来好消息！</par>
                                <par>只要<t color="red">图书管理人</t>和<t color="red">药剂师</t>加入我们这一方的话就是一次值得的斗争！</par>
                                <par></par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, asInitString(getQuestName()))

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
                                <par><event id="%%s">没问题！</event></par>
                                <par><event id="%%s">给我点儿考虑的时间。</event></par>
                            </layout>
                        ]=], SYS_EXIT, SYS_EXIT)
                    end,
                })
            ]], uid, asInitString(getQuestName()))
        end,

        quest_refuse_quest = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
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
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_accept_quest') ]=], uid)
                    end,

                    npc_deny = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>唉！</par>
                                <par>老天爷啊！真的丢下我不管了吗！？</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, getUID(), asInitString(getQuestName()))
        end,
    })
end
